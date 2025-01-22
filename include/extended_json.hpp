#pragma once

#include <fmt/chrono.h>

#include <chrono>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace extended_json {

struct value;

using list = std::vector<value>;

using record = std::unordered_map<std::string, value>;

using null = std::monostate;

using time = std::chrono::system_clock::time_point;
using duration = std::chrono::system_clock::duration;

struct value : std::variant<null, int64_t, double, std::string, time, duration,
                            list, record> {};

template <typename T>
concept structural = std::same_as<T, list> or std::same_as<T, record>;

template <typename T>
concept scalar = std::same_as<T, int64_t> or std::same_as<T, double>
                 or std::same_as<T, std::string> or std::same_as<T, time>
                 or std::same_as<T, duration>;

} // namespace extended_json
