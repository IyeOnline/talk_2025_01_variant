#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace json {

struct value;

using list = std::vector<value>;

using record = std::unordered_map<std::string, value>;

using null = std::monostate;

struct value : std::variant<null, int64_t, double, std::string, list, record> {
  using std::variant<null, int64_t, double, std::string, list, record>::variant;
};

template <typename T>
concept structural = std::same_as<T, list> or std::same_as<T, record>;

template <typename T>
concept scalar = std::same_as<T, int64_t> or std::same_as<T, double>
                 or std::same_as<T, std::string>;

struct parse_error {
  size_t level;
  std::string message;
};

/// very limited JSON parser.
auto parse(std::string_view) -> std::variant<value, parse_error>;

}; // namespace json
