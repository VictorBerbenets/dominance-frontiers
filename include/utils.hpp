#pragma once

#include <format>
#include <string>
#include <string_view>

namespace graphs {

namespace utils {

namespace fs = std::filesystem;

void dumpInPngFormat(fs::path PathToDotFile) {
  std::string DotCommand = "dot -Tpng ";
  DotCommand.append(PathToDotFile);
  DotCommand.append(" -o ");
  DotCommand.append(PathToDotFile.replace_extension(".png"));

  std::system(DotCommand.c_str());
}

template <typename... Args>
std::string formatPrint(std::string_view rt_fmt_str, Args &&...args) {
  return std::vformat(rt_fmt_str, std::make_format_args(args...));
}

} // namespace utils

} // namespace graphs
