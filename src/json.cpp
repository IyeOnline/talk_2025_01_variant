#include "json.hpp"

#include <fmt/format.h>

#include <cassert>
#include <charconv>
#include <optional>

#include "overload.hpp"
#include "util.hpp"

using namespace std::string_view_literals;

namespace json {

namespace {

constexpr static std::string_view whitespace = " \t\n";

auto strip_leading_ws(std::string_view source) -> std::string_view {
  size_t i = 0;
  for (; i < source.size() and whitespace.contains(source[i]); ++i) {
  }
  source.remove_prefix(i);
  return source;
}

template <typename T>
auto make_parse_error_for(size_t level) -> parse_error {
  return parse_error{
    level,
    fmt::format("Failed to parse value as {}", util::get_name<T>()),
  };
}

} // namespace

auto parse(std::string_view source) -> std::variant<value, parse_error> {
  auto result = value{};

  constexpr auto parser = overload{
    [](this const auto& self, std::string_view& source, value& out, size_t level)
      -> std::optional<parse_error> {
      source = strip_leading_ws(source);
      if (source.empty()) {
        out = null{};
        return {};
      }
      if (source.front() == '{') {
        auto& r = out.emplace<record>();
        return self(source, r, level);
      } else if (source.front() == '[') {
        auto& l = out.emplace<list>();
        return self(source, l, level);
      } else if (source.front() == '\"') {
        auto& s = out.emplace<std::string>();
        return self(source, s, level);
      }
      auto& i = out.emplace<int64_t>();
      auto is_int = self(source, i, level);
      if (is_int) {
        return {};
      }
      auto& d = out.emplace<double>();
      return self(source, d, level);
    },
    []<scalar T>(std::string_view& source, T& v, size_t level) -> std::optional<parse_error> {
      const auto [ptr, ec] = std::from_chars(source.begin(), source.end(), v);
      source.remove_prefix(static_cast<size_t>(ptr - source.data()));
      return make_parse_error_for<T>(level);
    },
    [](std::string_view& source, std::string& s, size_t level) -> std::optional<parse_error> {
      auto end = size_t{1};
      for (; end < source.size(); ++end) {
        end = source.find("\"", end);
        if (end == source.npos) {
          break;
        }
        auto escaping = 0;
        for (size_t i = end - 1; i > 0 and source[i] == '\\'; --i) {
          ++escaping;
        }
        if (escaping % 2 == 0) {
          break;
        }
      }
      if (end >= source.size()) {
        return parse_error{level, "Expected string to end"};
      }
      s = std::string{source.substr(1, end - 1)};
      source.remove_prefix(end + 1);
      return {};
    },
    [](this const auto& self, std::string_view& source, list& l, size_t level) -> std::optional<parse_error> {
      source = strip_leading_ws(source);
      if (source.empty() or source.front() != '[') {
        return parse_error{level, "expected list to start"};
      }
      source.remove_prefix(1);
      source = strip_leading_ws(source);
      if (source.empty()) {
        return parse_error{level, "expected list to end"};
      }
      if (source.front() == ']') {
        source.remove_prefix(1);
        return {};
      }
      while (not source.empty()) {
        auto& value = l.emplace_back();
        auto e = self(source, value, level + 1);
        if (e) {
          return e;
        }
        source = strip_leading_ws(source);
        if (source.front() == ']') {
          break;
        } else if (source.front() != ',') {
          return parse_error{level, "expected ',' or ']'"};
        }
        source.remove_prefix(1);
        source = strip_leading_ws(source);
      }
      assert(source.front() == ']');
      source.remove_prefix(1);
      return {};
    },
    [](this const auto& self, std::string_view& source, record& r, size_t level) -> std::optional<parse_error> {
      source = strip_leading_ws(source);
      if (source.empty() or source.front() != '{') {
        return parse_error{level, "expected record to start"};
      }
      source.remove_prefix(1);
      source = strip_leading_ws(source);
      if (source.empty()) {
        return parse_error{level, "expected record to end"};
      }
      if (source.front() == '}') {
        source.remove_prefix(1);
        return {};
      }
      while (not source.empty()) {
        auto key = std::string{};
        if (auto e = self(source, key, level)) {
          return parse_error{level, "failed to parse key: " + e->message};
        }

        source = strip_leading_ws(source);
        if (source.front() != ':') {
          return parse_error{level, "expected ':'"};
        }
        source.remove_prefix(1);

        const auto [it, _] = r.try_emplace(std::move(key));
        if (auto e = self(source, it->second, level + 1)) {
          return std::move(*e);
        }
        source = strip_leading_ws(source);
        if (source.front() == ',') {
          source.remove_prefix(1);
          source = strip_leading_ws(source);
        }
        if (source.front() == '}') {
          break;
        }
      }
      assert(source.front() == '}');
      source.remove_prefix(1);
      return {};
    },
  };

  auto error = parser(source, result, 0);
  if (error) {
    return std::move(*error);
  }

  source = strip_leading_ws(source);
  if (not source.empty()) {
    return parse_error{0, "source does not end at end of value"};
  }

  return std::move(result);
}

} // namespace json
