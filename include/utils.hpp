#pragma once

#include <format>
#include <string>
#include <string_view>

namespace graphs {

namespace utils {

template <typename... Args>
std::string formatPrint(std::string_view rt_fmt_str, Args &&...args) {
  return std::vformat(rt_fmt_str, std::make_format_args(args...));
}

} // namespace utils

} // namespace graphs
