#include <fmt/core.h>

#include <algorithm>

#include "extended_json.hpp"
#include "json.hpp"
#include "overload.hpp"

#define EXTENDED_JSON 0

auto printing_example(const auto& obj) -> void {
#if EXTENDED_JSON == 1
  using namespace extended_json
#else
  using namespace json;
#endif
    constexpr auto print
    = overload{
      [](const null&) -> void {
        fmt::print("null");
      },
      [](const std::string& v) -> void {
        fmt::print("\"{}\"", v);
      },
      [](const scalar auto& v) -> void {
        fmt::print("{}", v);
      },
      [](this auto self, const list& l) -> void {
        fmt::print("[ ");
        bool first = true;
        for (const auto& v : l) {
          if (not first) {
            fmt::print(", ");
          }
          self(v);
          first = false;
        }
        fmt::print(" ]");
      },
      [](this auto self, const record& r) -> void {
        fmt::print("{{\n");
        for (const auto& [k, v] : r) {
          fmt::print("\"{}\": ", k);
          self(v);
          fmt::print(",\n");
        }
        fmt::print("}}\n");
      },
      [](this auto self, const value& v) -> void {
        std::visit(self, v);
      },
    };

  print(obj);

  fmt::print("\n");
}

auto counting_example(const auto& obj) -> void {
#if EXTENDED_JSON == 1
  using namespace extended_json
#else
  using namespace json;
#endif

    constexpr auto count_values
    = overload{
      [](const null&) -> size_t {
        return 0;
      },
      [](const std::string&) -> size_t {
        return 1;
      },
      [](const scalar auto&) -> size_t {
        return 1;
      },
      [](this auto self, const list& l) -> size_t {
        auto sum = size_t{};
        for (const auto& v : l) {
          sum += self(v);
        }
        return sum;
      },
      [](this auto self, const record& r) -> size_t {
        auto sum = size_t{};
        for (const auto& [k, v] : r) {
          sum += self(v);
        }
        return sum;
      },
      [](this auto self, const value& v) -> size_t {
        return std::visit(self, v);
      },
    };

  const auto fields = count_values(obj);

  fmt::print("obj has {} values", fields);
}

int main() {
  using namespace std::chrono_literals;

#if EXTENDED_JSON == 1
  using namespace extended_json;
  auto obj = value{
    record{
      {"string", {"value"}},
      {"time", {std::chrono::system_clock::now()}},
      {"duration", {1s}},
    },
  };
#else
  using namespace json;
  std::string_view source = R"("hello\"world")";
  // auto obj = value{
  //   record{
  //     {"int64", {1}},
  //     {"double", {1.1}},
  //     {"string", {"value"}},
  //   },
  // };
  auto obj = value{};
  std::visit(overload{
               [&](value&& v) {
                 obj = std::move(v);
               },
               [](const parse_error& e) {
                 fmt::print("error at depth {}: {}\n", e.level, e.message);
               },
             },
             parse(source));
#endif
  printing_example(obj);
  counting_example(obj);
}
