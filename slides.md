<!--- Presentation created using 'markdown-slides' --->
<!--- https://gitlab.com/da_doomer/markdown-slides --->
<!--- mdslides --include img slides.md --->
[comment]: # (THEME = night)
[comment]: # (CODE_THEME = base16/zenburn)
[comment]: # (slideNumber: true)

## `std::variant` <br/> A practical application

<br/>
<br/>

Johannes Misch

johannes.misch@tenzir.com

[![Tenzir](imgs/tenzir-white.svg)](https://www.tenzir.com)

[comment]: # (!!!)

## The Goal

Lets store any JSON value:
```json [|1,13|2|3|4-6|7-12]
{
	"name": "Johannes",
	"age": 29,
	"groups": [
		"C++ User Group Frankfurt"
	],
	"jobs" : [
		{
			"company": "Tenzir",
			"title": "C++ Engineer"
		}
	]
}
```
```f90 [|1|2|3]
value := null OR int OR float OR string OR list OR record
record := { string : value , ... }
list := [ value , ... ]
```

[comment]: # (!!!)

## C++ has Polymorphism!

```cpp []
std::unique_ptr<Value> json::load_file(std::string_view);
auto me = json::load_file("johannes.json");
me->print();
```
```cpp [1-4|3|6|9|21|25-28]
struct Value {
	virtual ~Value() = default;
	virtual void print() const = 0;
};
struct Null : Value {
	virtual void print() const override;
};
struct Integer : Value {
	uint64_t data;
	virtual void print() const override;
};
struct Float : Value {
	double data;
	virtual void print() const override;
};
struct String : Value {
	std::string data;
	virtual void print() const override;
};
struct List : Value {
	std::vector<std::unique_ptr<Value>> data;
	virtual void print() const override;
};
struct Record : Value {
	std::unordered_map<
		std::string,
		std::unique_ptr<Value>
	> data;
	virtual void print() const override;
};



// empty space for presentation framework
```
<!-- .element: class="fragment" data-fragment-index="1" -->

[comment]: # (!!!)

## C++ has Templates!

```cpp [|4-7|11|14|8-9]
struct Value {
	virtual ~Value() = default;
	virtual void print() const = 0;
};
template<typename T>
struct Impl : Value {
	T data;
	// This is actually not straightforward
	virtual void print() const override;
};
using None = Impl<std::monostate>; // :|
using Integer = Impl<uint64_t>;
using String = Impl<std::string>;
using List = Impl<std::vector<std::unique_ptr<Value>>>;
using Record = Impl<std::unordered_map<std::string,
	std::unique_ptr<Value>>>;
```

[comment]: # (!!!)

## Problems

Lets read a simple JSON file:

```json title="my_config.json"
{
	"key": "value"
}
```
And access a value:
```cpp
std::unique_ptr<Value> my_config = json::load_file("ex.json");
std::string value = ... "key" ... ?
```

[comment]: # (!!!)

## Solutions?

### New virtual member function
```cpp [1]
auto value = my_config->at<std::string>("key");
```
### Expose details
```cpp [|1|2|3]
auto rec = dynamic_cast<Record*>( my_config.get() );
auto value_erased = rec->data.at("key").get();
auto value = dynamic_cast<String*>( value_erased ).data;
```

[comment]: # (!!! data-auto-animate)

## Problems

- Extension
	- New virtual function for every feature
	- Exposed implementation details
- Concrete type is 'erased' at runtime
<!-- .element: class="fragment" data-fragment-index="1" -->
	- `dynamic_cast` and manual checks
<!-- .element: class="fragment" data-fragment-index="1" -->
	- error prone
<!-- .element: class="fragment" data-fragment-index="1" -->
- We don't actually need the fully open set polymorphism
<!-- .element: class="fragment" data-fragment-index="2" -->
- Inefficient Memory Usage & Structure
<!-- .element: class="fragment" data-fragment-index="3" -->
	- Pointers to Pointers
<!-- .element: class="fragment" data-fragment-index="3" -->
	- Pointer + vtable pointer to store a single int
<!-- .element: class="fragment" data-fragment-index="3" -->

[comment]: # (!!!)

## C++ has `union`
[comment]: # (!!! data-auto-animate)
## C++ has `union`

* Can store one alternative at a time
* Unsafe access
<!-- .element: class="fragment" data-fragment-index="1" -->
* Manually keep track of the active member
<!-- .element: class="fragment" data-fragment-index="2" -->
* Manual lifetime management for non-trivial members
<!-- .element: class="fragment" data-fragment-index="3" -->

=> "*`union` is evil*".
<!-- .element: class="fragment" data-fragment-index="4" -->

[comment]: # (!!! data-auto-animate)


## C++ has `std::variant`
[comment]: # (!!! data-auto-animate)
## C++ has `std::variant`

* "Typesafe union"
* Knows its active member
<!-- .element: class="fragment" data-fragment-index="1" -->
* Manages the lifetime, supporting all operations of a regular type.
<!-- .element: class="fragment" data-fragment-index="2" -->
* No dynamic memory:
<!-- .element: class="fragment" data-fragment-index="3" -->
`constexpr`
<!-- .element: class="fragment" data-fragment-index="3" -->
* C++17
<!-- .element: class="fragment" data-fragment-index="4" -->
	- Can be fully implemented in C++11 (boost)
<!-- .element: class="fragment" data-fragment-index="5" -->
	- Possible for trivial types in C++98
<!-- .element: class="fragment" data-fragment-index="6" -->

[comment]: # (!!! data-auto-animate)

## Implementation

```cpp [|1|3-4|5|8-9|10-11|12|14]
template<typename ... Ts>
class variant
{ // Illustration purposes only
	union { Ts ... alternatives_ };
	size_t index_;

public:
	variant( const variant& );
	variant( variant&& );
	variant& operator=( const variant& );
	variant& operator=( variant&& );
	~variant();

	size_t index() const { return index_; }
};
```
[comment]: # (!!!)

## `variant` in practice

[comment]: # (!!!)

### Construction

Directly constructible from alternative:
```cpp [3]
#include <variant>

std::variant<int,std::string> my_variant = 42;
```
In-place construction:
<!-- .element: class="fragment" data-fragment-index="1" -->
```cpp [|1|2|3]
std::variant<std::vector<int>,std::string> my_variant{
	std::in_place_type<std::string>,
	42,
};
```
<!-- .element: class="fragment" data-fragment-index="1" -->

[comment]: # (!!!)

### Ugly Secret

Alternatives do not have to be unique:
```cpp [|1|2]
std::variant<int,int,int> my_variant{
	std::in_place_index<1>,
	42,
};
```

If alternatives are not unique, we can only refer to them by index.

Avoid repeated alternatives: <br/> Distinct types convey meaning!
<!-- .element: class="fragment" data-fragment-index="1" -->

[comment]: # (!!!)

### Checking contents

Using the index
```cpp [3]
std::variant<int,std::string> my_variant = 42;

bool holds_int = my_variant.index() == 0;
```

`std::holds_alternative`:
<!-- .element: class="fragment" data-fragment-index="1" -->
```cpp [3]
std::variant<int,std::string> my_variant = 42;

bool holds_int = std::holds_alternative<int>(my_variant);
```
<!-- .element: class="fragment" data-fragment-index="1" -->

[comment]: # (!!!)

### Assignment

```cpp [|1|3|4|5]
std::variant<int,std::string> my_variant = 42;

my_variant = 0;
my_variant = "Hello World!";
my_variant = "Hello Frankfurt!";

assert( std::holds_alternative<std::string>(my_variant) );
```
[comment]: # (!!!)

### Emplacing

```cpp [|1|3]
std::variant<int,std::string> my_variant = 42;

my_variant.emplace<std::string>( 42 );

assert( std::holds_alternative<std::string>(my_variant) );
```

[comment]: # (!!!)

### Access via `std::get`

Obtain a reference to the held object:

```cpp
if ( std::holds_alternative<int>( my_variant ) ) {
```
```cpp
	int& i = std::get<int>( my_variant );
```
<!-- .element: class="fragment" data-fragment-index="1" -->
```cpp
	const int& i = std::get<int>( std::as_const(my_variant) );
```
<!-- .element: class="fragment" data-fragment-index="2" -->
```cpp
	int&& i = std::get<int>( std::move(my_variant) );
}
```
<!-- .element: class="fragment" data-fragment-index="3" -->

[comment]: # (!!!)

### Access failures

Accessing inactive alternative throws:

```cpp [|1,4|5]
std::variant<int,std::string> my_variant = 42;

try {
	std::get<std::string>( my_variant );
} catch ( const std::bad_variant_access& e ) {
	fmt::print("{}",e.what());
}
```

[comment]: # (!!!)

### `std::get_if`

Combined check and access:
```cpp [|1|2]
if ( auto* ptr = std::get_if<int>( &my_variant ) ) {
	fmt::print("variant holds `int`: {}\n",*ptr);
} else {
	fmt::print("variant does not hold `int`.");
}
```

[comment]: # (!!!)

### `std::visit`
[comment]: # (!!! data-auto-animate)
### `std::visit`

"runtime overload resolution"

```cpp [|3,6|4|5|3-6]
std::variant<std::string,int> my_variant = "Hello World";

std::visit(
	[](const auto& v){ fmt::print("{}\n",v); },
	my_variant
);
```

Invokes functor with currently held alternative.

[comment]: # (!!! data-auto-animate)
### `std::visit`
* Best match for each alternative is picked based on overload resolution
  * Functor must be invocable with all alternatives
<!-- .element: class="fragment" data-fragment-index="1" -->
* Return type must match between all invocations
<!-- .element: class="fragment" data-fragment-index="2" -->
  * Return type may be <!-- .element: class="fragment" data-fragment-index="3" --> `void`
<!-- .element: class="fragment" data-fragment-index="3" -->
* At runtime the function for the held alternative is invoked
<!-- .element: class="fragment" data-fragment-index="4" -->

[comment]: # (!!! data-auto-animate)

### Multi-visit

```cpp [|1-2|4|7|]
std::variant<double,int> a = 100;
std::variant<int,double> b = 1.0;

auto visitor = []( const auto x, const auto y ) -> double {
	return x + y;
};
auto sum = std::visit( vistor, a, b );
```

[comment]: # (!!!)

### Visiting an overload set

```cpp [|3-4]
std::variant<int,double> my_variant = 42;

void f( int );
void f( double );

std::visit( &f, my_variant )
```
```cpp
<source>:6:1: error: no matching function for call to 'visit(<unresolved overloaded function type>, std::variant<int, double>)'
    6 |     std::visit( &f, my_variant );
      |     ~~~~~~~~~~^~~~~~~~~~~~~~~~~~
```
<!-- .element: class="fragment" data-fragment-index="1" -->
```cpp
'visit(<unresolved overloaded function type>, std::variant<int, double>)'
```
<!-- .element: class="fragment" data-fragment-index="2" -->

```cpp []
std::visit(
  []<typename T>( T&& v){ return f(std::forward<T>(v)); },
  my_variant
);
```
<!-- .element: class="fragment" data-fragment-index="3" -->

[comment]: # (!!!)

### The `overload` pattern

```cpp [|1|2|3|5-7|5]
template<typename ... Ts>
struct overload : Ts... {
	using Ts::operator() ...;
};
// CTAD guide for C++17:
template<typename ... Ts>
struct overload( Ts ... ) -> overload<Ts...>;
```
<!-- .element: class="fragment" data-fragment-index="1" -->
```cpp [|3|4|5]
std::variant<std::string,int> my_variant = "Hello World";

std::visit( overload{
	[]( const std::string& s){ fmt::print("tring: {}",s); },
	[]( const int& i){ fmt::print("int: {}",i); },
}, my_variant );
```
<!-- .element: class="fragment" data-fragment-index="1" -->

[comment]: # (!!! data-auto-animate)

### The `overload` pattern

* Easy way to ad-hoc craft an overload set
* 'match' in other languages.
* More complex versions are possible
  * Free functions
  * Member functions
  * Member variables

[comment]: # (!!! data-auto-animate)

## Back to JSON

[comment]: # (!!!)

### Define JSON

```cpp [|1|2|3|4|5,12|6-11|6]
struct value;
using list = std::vector<value>;
using record = std::unordered_map<std::string, value>;
using null = std::monostate;
struct value : std::variant<
	null,
	int64_t,
	double,
	std::string,
	list,
	record
> {};
```
* Inheriting from standard variant requires C++20!
  * Allows us to directly `visit` `value`.
* **Iff** first alternative is default constructible, it is the default state

[comment]: # (!!!)

### Create an object:

```cpp [|1,6|2,5|3-4]
auto obj = value{
  record{
    { "key", { "value" } },
    { "key2", { null{} } },
  },
};
```

[comment]: # (!!!)

### A simple visit

Let's obtains the length of a type-erased string:

```cpp [|1-3|5,10|6-7|8-9|12|13-15|]
auto str = value{
  "Hello World",
};

constexpr auto string_length = overload{
  [&](const std::string& s) -> std::optional<size_t>
  { return s.length(); },
  [&](const auto&) -> std::optional<size_t>
  { return std::nullopt; },
};

auto l = std::visit( string_length, str );
if ( l ) {
  fmt::print("The string has length {}\n",*l);
}
```

Advice: "Fallback" cases should be `const auto&`.

[comment]: # (!!!)
## Interlude:
### Concepts
[comment]: # (!!! data-auto-animate)

## Interlude:
### Concepts

Concepts (C++20) allow us to *constrain* a template:

```cpp [|1,3|2]
template<typename T>
  requires std::integral<T>
void func( T t );
```
[comment]: # (!!! data-auto-animate)

## Interlude:
### Concepts

Concepts (C++20) allow us to *constrain* a template:

```cpp [2]
template<typename T>
  requires std::integral<T>
void func( T t );
```
```cpp [1]
template<std::integral T>
void func( T t );
```
<!-- .element: class="fragment" data-fragment-index="1" -->
```cpp []
void func( std::integral auto t );
```
<!-- .element: class="fragment" data-fragment-index="2" -->

[comment]: # (!!! data-auto-animate)

### A concept of JSON

```cpp [|1-5|6-10]
template <typename T>
concept structural =
  std::same_as<T, list> or
  std::same_as<T, record>;

template <typename T>
concept scalar =
  std::same_as<T, int64_t> or
  std::same_as<T, double> or
  std::same_as<T, std::string>;
```

[comment]: # (!!!)

## Interlude:
### Explicit object parameter

[comment]: # (!!! data-auto-animate)

## Interlude:
### Explicit object parameter

Explicit object parameters (C++23) make the implicit `this` pointer explicit

```cpp [|2|3|]
struct S{
  auto func( this auto&& self ) {
    fmt::print( "{}", get_name<decltype(self)>() );
  }
};
```
```cpp [|2|3|4]
S s;
s.f();                // 'S&'
std::move(s).f();     // 'S&&'
std::as_const(s).f(); // 'const S&'
```

[comment]: # (!!! data-auto-animate)

## Printing JSON
[comment]: # (!!! data-auto-animate)
## Printing JSON

```cpp [|1|2|3|4|3,4]
constexpr auto print = overload{
  [](const null&) -> void { fmt::print("null"); },
  [](const std::string& v) -> void { fmt::print("\"{}\"", v); },
  [](const scalar auto& v) -> void { fmt::print("{}", v); },
```

[comment]: # (!!! data-auto-animate)

## Printing JSON

```cpp [3,5,7,9,11|3,11|5,9|6|7|1,7]
constexpr auto print = overload{
  //...
  [](this auto self, const record& r) -> void {
    fmt::print("{{\n");
    for (const auto& [k, v] : r) {
      fmt::print("\"{}\": ", k);
      self(v);
      fmt::print(",\n");
    }
    fmt::print("}}\n");
  },
```

* Explicit object parameter
  * Useful for recursive lambdas
  * Also does CRTP for us
* Still possible in C++11, but not as nice

[comment]: # (!!! data-auto-animate)

## Printing JSON

```cpp [|3|4]
constexpr auto print = overload{
  //...
  [](this auto self, const value& v) -> void {
    std::visit(self, v);
  },
};
```

[comment]: # (!!! data-auto-animate)

## Printing JSON

```cpp [|14|1|7|7,4|8|8,3|9|9,4|9,2|]
constexpr auto print = overload{
  [](const std::string& v) -> void { /*...*/ },
  [](this auto self, const record& v) -> void { /*...*/ },
  [](this auto self, const value& v) -> void { /*...*/ },
};

auto obj = value{
  record{
    { "key", { "value" } },
    { "key2", { null{} } },
  },
};

print(obj);
```
```json
{
"key2": null,
"key": "value",
}
```
<!-- .element: class="fragment" data-fragment-index="1" -->

[comment]: # (!!! data-auto-animate)
## Extending the typeset
[comment]: # (!!! data-auto-animate)

## Extending the typeset

```cpp [|2,3|9,10]
/* ... */;
using time = std::system_clock::time_point;
using duration = std::system_clock::duration;
struct value : std::variant<
	null,
	int64_t,
	double,
	std::string,
	time,
	duration,
	list,
	record
> {};
```
[comment]: # (!!! data-auto-animate)

## Extending the typeset

```cpp [2,6,7]
template <typename T>
concept scalar =
  std::same_as<T, int64_t> or
  std::same_as<T, double> or
  std::same_as<T, std::string> or
  std::same_as<T, time> or
  std::same_as<T, duration>;
```

[comment]: # (!!! data-auto-animate)

## Printing "JSON"

Print the extended JSON using the same `print`:

```cpp [|4,5|9]
auto obj = value{
  record{
    {"string", {"value"}},
    {"time", {std::chrono::system_clock::now()}},
    {"duration", {1s}},
  },
};

print(obj);
```
```json
{
"duration": 1000000000ns,
"time": 2025-01-12 23:36:56,
"string": "value",
}
```

[comment]: # (!!!)

## Adding functionality

```cpp [|1,9|2|3|4,5|6-8]
constexpr auto count_values = overload{
  [](const null&) -> size_t { return 0; },
  [](const scalar auto&) -> size_t { return 1; },
  [](this auto self, const list& l) -> size_t { /* ... */ },
  [](this auto self, const record& r) -> size_t { /*...*/ },
  [](this auto self, const value& v) -> size_t {
    return std::visit(self, v);
  },
};

const auto values = count_values(obj);
```

[comment]: # (!!!)

## The End

* `std::variant`
  * closed set polymorphism
  * type safe union
* `std::holds_alternative` to check contents
* `std::get`/`std::get_if` to access contents
* `std::visit` for powerful operations
  * C++20 constraints
  * C++23 explicit object parameters
