#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "directed_graph.hpp"
#include "dominance_tree_graph.hpp"
#include "dominance_frontier_graph.hpp"
#include "graph_generator.hpp"

namespace {

namespace fs = std::filesystem;

using namespace std::literals::string_literals;

using value_type = int;
using DGT = graphs::DirectedGraph<value_type>;
using DTT = graphs::DomTreeGraph<value_type>;
using DGBT = graphs::DirGraphBuilder;
using OptIter = typename std::vector<std::string>::iterator;
using OptMap = std::unordered_map<std::string_view, std::string>;

std::vector<std::pair<std::string, std::string>>
getGraphEdges(std::ifstream &Ifs) {
  static constexpr std::string_view Arrow = " --> ";

  std::vector<std::pair<std::string, std::string>> Edges;
  std::string Line;
  while (std::getline(Ifs >> std::ws, Line)) {

    auto FirstPos = Line.find_first_of(Arrow);
    Edges.emplace_back(Line.substr(0, FirstPos),
                       Line.substr(FirstPos + Arrow.size(), std::string::npos));
  }

  return Edges;
}

struct CommandContext final {
  OptMap &OM;
  const std::string_view Com;
};

namespace opts {

constexpr std::string_view Path = "--path";
constexpr std::string_view NumNodes = "--num-nodes";
constexpr std::string_view NumEdges = "--num-edges";
constexpr std::string_view NodeColor = "--node-color";
constexpr std::string_view EdgeColor = "--edge-color";
constexpr std::string_view NodeShape = "--node-shape";
constexpr std::string_view EdgeShape = "--edge-shape";
constexpr std::string_view FileName = "--file-name";
constexpr std::string_view GraphName = "--graph-name";
constexpr std::string_view NodeName = "--node-name";
constexpr std::string_view Arg = "--arg";

}; // namespace opts

namespace coms {

constexpr std::string_view H = "-h";
constexpr std::string_view Help = "-help";
constexpr std::string_view Cfg = "-g=cfg";
constexpr std::string_view CfgTxt = "-g=cfg-txt";
constexpr std::string_view CfgDot = "-g=cfg-dot";
constexpr std::string_view CfgPng = "-g=cfg-png";
constexpr std::string_view DomTree = "-g=dom-tree";
constexpr std::string_view DomTreeDot = "-g=dom-tree-dot";
constexpr std::string_view DomTreePng = "-g=dom-tree-png";

}; // namespace coms

enum class ComCodes : char {
  Help,
  CfgDot,
  CfgPng,
  Cfg,
  CfgTxt,
  DomTreeDot,
  DomTreePng,
  DomTreeTxt,
  DomTree
};

ComCodes getComCode(std::string_view Command) {
  static std::unordered_map<std::string_view, ComCodes> ComCodesMap{
      {coms::H, ComCodes::Help},
      {coms::Help, ComCodes::Help},
      {coms::CfgDot, ComCodes::CfgDot},
      {coms::CfgPng, ComCodes::CfgPng},
      {coms::Cfg, ComCodes::Cfg},
      {coms::DomTree, ComCodes::DomTree},
      {coms::CfgTxt, ComCodes::CfgTxt},
      {coms::DomTreeDot, ComCodes::DomTreeDot},
      {coms::DomTreePng, ComCodes::DomTreePng},
  };

  auto FindIt = ComCodesMap.find(Command);
  if (FindIt == ComCodesMap.end())
    throw std::runtime_error{
        std::string(Command).append(" is not available command. Try -help")};
  return FindIt->second;
}

void printHelp(std::ostream &Os = std::cout) {
  Os << std::string(47, '-') << "help" << std::string(49, '-') << std::endl;
  Os << "|-"
     << "To generate CFG in choosen format use next commands:" << std::endl;
  Os << "|\t" << "-g=cfg-dot\n|\t-g=cfg-png\n|\t-g=cfg-txt\n|\t-g=cfg"
     << std::endl;
  Os << "|-"
     << "To generate dominance tree in choosen format use next commands:"
     << std::endl;
  Os << "|\t" << "-g=dom-tree-dot\n|\t-g=dom-tree-png\n|\t-g=dom-tree"
     << std::endl;
  Os << "|-"
     << "Note: commands -g=cfg and -g=dom-tree generate all graph formats."
     << std::endl;
  Os << "|-" << "Options:" << std::endl;
  Os << "|\t"
     << "--arg=<>        - generate graph from txt file with graph "
        "representation."
     << std::endl;
  Os << "|\t" << "--path=<>       - path to create files." << std::endl;
  Os << "|\t" << "--graph-name=<> - set graph name." << std::endl;
  Os << "|\t" << "--num-nodes=<>  - set number of nodes." << std::endl;
  Os << "|\t" << "--num-edges=<>  - set the limit on the number of node edges."
     << std::endl;
  Os << "|\t" << "--node-color=<> - set node color (lightblue is default)."
     << std::endl;
  Os << "|\t" << "--edge-color=<> - set edge color (red is default)."
     << std::endl;
  Os << "|\t"
     << "--node-shape=<> - set node shape (square is default). Check "
        "graphviz.org "
        "for more."
     << std::endl;
  Os << "|\t"
     << "--edge-shape=<> - set edge shape (vee is default). Check graphviz.org "
        "for "
        "more."
     << std::endl;
  Os << "|\t"
     << "--file-name=<>  - set name for generated file(s) (name graph setted "
        "as default)."
     << std::endl;
  Os << "|\t" << "--node-name=<>  - set name for nodes. BB setted as default."
     << std::endl;
  Os << "|-"
     << "Note: you can use RGB format for color option (e.g. "
        "--node-color=#ffffff)."
     << std::endl;
  Os << std::string(100, '-') << std::endl;
}

int getOptInt(std::string_view Opt, std::string_view Arg) {
  if (int Num = std::stoi(std::string(Arg)); Num < 0)
    throw std::runtime_error{
        std::string(Opt).append(": invalid argument "s.append(Arg))};
  else
    return Num;
}

void checkFilePath(const fs::path &Path) {
  if (!fs::exists(Path))
    throw std::runtime_error{Path.string().append(" is invalid path")};
}

fs::path getPath(std::string_view PathStr) {
  fs::path Path{PathStr};
  checkFilePath(Path);

  return Path;
}

fs::path generateTxtFormatGraph(OptMap &OM) {
  fs::path FilePath;
  if (FilePath = OM[opts::Arg]; FilePath.string().empty()) {
    FilePath = getPath(OM[opts::Path])
                   .append(OM[opts::FileName])
                   .replace_extension(".txt");
    std::ofstream TxtFile{FilePath};
    DGBT::generateGraph(TxtFile, getOptInt(opts::NumNodes, OM[opts::NumNodes]),
                        getOptInt(opts::NumEdges, OM[opts::NumEdges]),
                        OM[opts::NodeName]);
  } else {
    checkFilePath(FilePath);
  }
  return FilePath;
}

template <typename GraphType>
  requires std::derived_from<GraphType, DGT> &&
           requires(GraphType Gr, std::ofstream Os) {
             { Gr.dumpInDotFormat(Os) };
           }
fs::path generateDotFormatGraph(CommandContext &CC) {
  auto FilePath = generateTxtFormatGraph(CC.OM);
  std::ifstream TxtFile{FilePath};
  auto Edges = getGraphEdges(TxtFile);
  GraphType G(Edges.cbegin(), Edges.cend());
  if (CC.Com != coms::Cfg && CC.OM[opts::Arg].empty())
    fs::remove(FilePath);
  std::ofstream DotFile{FilePath.replace_extension(".dot")};
  G.dumpInDotFormat(DotFile, CC.OM[opts::GraphName], CC.OM[opts::NodeShape],
                    CC.OM[opts::NodeColor], CC.OM[opts::EdgeShape],
                    CC.OM[opts::EdgeColor]);
  DotFile.close();
  return FilePath;
}

template <typename GraphType>
  requires std::derived_from<GraphType, DGT>
void generatePngFormatGraph(CommandContext &CC) {
  auto DotFilePath = generateDotFormatGraph<GraphType>(CC);
  graphs::utils::dumpInPngFormat(DotFilePath);
  if (CC.Com != coms::Cfg && CC.Com != coms::DomTree)
    fs::remove(DotFilePath);
  std::system(
      "display "s.append(DotFilePath.replace_extension(".png")).c_str());
}

template <typename GraphType>
  requires std::derived_from<GraphType, DGT>
void generateFullExtensionGraph(CommandContext &CC) {
  generatePngFormatGraph<GraphType>(CC);
}

template <std::input_iterator Iter>
void printInvalidOptions(Iter Begin, Iter end, std::ostream &Os = std::cout) {
  Os << "Error: invalid option"s.append(end == std::next(Begin) ? ":" : "s:")
     << std::endl;
  std::copy(Begin, end, std::ostream_iterator<std::string>(Os, "\n"));
  Os << "Try run with -h" << std::endl;
}

static constexpr std::string_view DefFileName = "graph";

OptMap OptsMap{{opts::Path, "."},
               {opts::NumNodes, std::to_string(DGBT::DefNodeNum)},
               {opts::NumEdges, std::to_string(DGBT::DefEdgeNum)},
               {opts::NodeColor, std::string(DGT::DefNodeColor)},
               {opts::EdgeColor, std::string(DGT::DefEdgeColor)},
               {opts::NodeShape, std::string(DGT::DefNodeShape)},
               {opts::EdgeShape, std::string(DGT::DefEdgeShape)},
               {opts::GraphName, std::string(DGT::DefGraphName)},
               {opts::FileName, std::string(DefFileName)},
               {opts::NodeName, std::string(DGBT::DefNodeName)},
               {opts::Arg, {}}};

} // namespace

int main(int args, char **argv) {
  if (args < 2) {
    throw std::runtime_error{"input error: expected command. Try run with"
                             " -h\n"};
  }

  std::vector<std::string> ErrorOpts;
  std::vector<std::string> OptionSet(std::next(argv), argv + args);
  for (auto &OptStr : OptionSet | std::views::drop(1)) {
    auto Delimetr = OptStr.find_first_of('=');
    auto Opt = OptStr.substr(0, Delimetr);
    if (Delimetr == OptStr.npos || !OptsMap.contains(Opt))
      ErrorOpts.push_back(std::move(Opt));
    else
      OptsMap[Opt] = std::move(OptStr.substr(++Delimetr));
  }

  if (!ErrorOpts.empty()) {
    printInvalidOptions(ErrorOpts.cbegin(), ErrorOpts.cend());
    return 1;
  }

  CommandContext CC{OptsMap, OptionSet.front()};
  switch (getComCode(OptionSet.front())) {
  case ComCodes::Help:
    printHelp();
    break;
  case ComCodes::Cfg:
    generateFullExtensionGraph<DGT>(CC);
    break;
  case ComCodes::CfgTxt:
    generateTxtFormatGraph(CC.OM);
    break;
  case ComCodes::CfgDot:
    generateDotFormatGraph<DGT>(CC);
    break;
  case ComCodes::CfgPng:
    generatePngFormatGraph<DGT>(CC);
    break;
  case ComCodes::DomTree:
    generateFullExtensionGraph<DTT>(CC);
    break;
  case ComCodes::DomTreeDot:
    generateDotFormatGraph<DTT>(CC);
    break;
  case ComCodes::DomTreePng:
    generatePngFormatGraph<DTT>(CC);
    break;
  default:
    throw std::runtime_error{std::string(CC.Com).append(
        " is not available command. Try -help, -h.")};
  }
}
