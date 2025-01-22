#pragma once
#include <string_view>

namespace util {

template <typename T>
consteval static std::string_view get_name() {
#if defined _WIN32
  constexpr std::string_view s = __FUNCTION__;
  const auto begin_search = s.find_first_of("<");
  const auto space = s.find(' ', begin_search);
  const auto begin_type = space != s.npos ? space + 1 : begin_search + 1;
  const auto end_type = s.find_last_of(">");
  return s.substr(begin_type, end_type - begin_type);
#elif defined __GNUC__
  constexpr std::string_view s = __PRETTY_FUNCTION__;
  constexpr std::string_view t_equals = "T = ";
  const auto begin_type = s.find(t_equals) + t_equals.size();
  const auto end_type = s.find_first_of(";]", begin_type);
  return s.substr(begin_type, end_type - begin_type);
#endif
}

} // namespace util
