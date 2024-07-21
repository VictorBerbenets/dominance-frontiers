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
#include "dominance_frontier_graph.hpp"
#include "dominance_tree_graph.hpp"
#include "graph_generator.hpp"

namespace {

namespace fs = std::filesystem;

using namespace graphs;
using namespace graphs::utils;
using namespace std::literals::string_literals;

using value_type = int;
using DGT = DirectedGraph<value_type>;
using DTT = DomTreeGraph<value_type>;
using DJGT = DomJoinGraph<value_type>;
using DFT = DomFrontierGraph<value_type>;
using DGBT = DirGraphBuilder;
using OptIter = typename std::vector<std::string>::iterator;
using OptMap = std::unordered_map<std::string_view, std::string>;

std::vector<EdgeType> getGraphEdges(std::ifstream &Ifs) {
  static constexpr std::string_view Arrow = " --> ";

  std::vector<EdgeType> Edges;
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
constexpr std::string_view JoinGraph = "-g=join-graph";
constexpr std::string_view JoinGraphDot = "-g=join-graph-dot";
constexpr std::string_view JoinGraphPng = "-g=join-graph-png";
constexpr std::string_view DomFrontier = "-g=dom-frontier";
constexpr std::string_view DomFrontierDot= "-g=dom-frontier-dot";
constexpr std::string_view DomFrontierPng = "-g=dom-frontier-png";

}; // namespace coms

constexpr std::string_view DefFileName = "graph";
constexpr int ErrorInputCode = 0x777;

std::vector<std::string> InputErrors;

enum class ComCodes : char {
  Help,
  CfgDot,
  CfgPng,
  Cfg,
  CfgTxt,
  DomTreeDot,
  DomTreePng,
  DomTreeTxt,
  DomTree,
  JoinGraph,
  JoinGraphDot,
  JoinGraphPng,
  DomFrontier,
  DomFrontierDot,
  DomFrontierPng
};

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

std::unordered_map<std::string_view, ComCodes> ComCodesMap{
    {coms::H, ComCodes::Help},
    {coms::Help, ComCodes::Help},
    {coms::CfgDot, ComCodes::CfgDot},
    {coms::CfgPng, ComCodes::CfgPng},
    {coms::Cfg, ComCodes::Cfg},
    {coms::DomTree, ComCodes::DomTree},
    {coms::CfgTxt, ComCodes::CfgTxt},
    {coms::DomTreeDot, ComCodes::DomTreeDot},
    {coms::DomTreePng, ComCodes::DomTreePng},
    {coms::JoinGraphPng, ComCodes::JoinGraphPng},
    {coms::JoinGraphDot, ComCodes::JoinGraphDot},
    {coms::JoinGraph, ComCodes::JoinGraph},
    {coms::DomFrontierDot, ComCodes::DomFrontierDot},
    {coms::DomFrontierPng, ComCodes::DomFrontierPng},
    {coms::DomFrontier, ComCodes::DomFrontier},
};

ComCodes getComCode(std::string_view Command) {
  auto FindIt = ComCodesMap.find(Command);
  assert(FindIt != ComCodesMap.end());
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
     << "To generate dominance join graph in choosen format use next commands:"
     << std::endl;
  Os << "|\t" << "-g=join-graph-dot\n|\t-g=join-graph-png\n|\t-g=join-graph"
     << std::endl;
  Os << "|-"
     << "To generate dominance frontier graph in choosen format use next commands:"
     << std::endl;
  Os << "|\t" << "-g=dom-frontier-dot\n|\t-g=dom-frontier-png\n|\t-g=dom-frontier"
     << std::endl;
  Os << "|-"
     << "Note: commands -g=cfg, -g=dom-tree, -g=join-graph, -g=dom-frontier "
     "generate all graph formats." << std::endl;
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

template <typename GraphType>
concept DotGraphTipe = std::same_as<GraphType, DJGT> ||
    std::same_as<GraphType, DFT> ||
    (std::derived_from<GraphType, DGT> &&requires(GraphType Gr,
                                                  std::ofstream Os) {
      {Gr.dumpInDotFormat(Os)};
    });

fs::path generateTxtFormatGraph(OptMap &OM) {
  fs::path FilePath;
  if (FilePath = OM[opts::Arg]; FilePath.string().empty()) {
    FilePath = fs::path(OM[opts::Path])
                   .append(OM[opts::FileName])
                   .replace_extension(".txt");
    std::ofstream TxtFile{FilePath};
    DGBT::generateGraph(TxtFile, std::stoi(OM[opts::NumNodes]),
                        std::stoi(OM[opts::NumEdges]), OM[opts::NodeName]);
  }
  return FilePath;
}

template <DotGraphTipe GraphType>
fs::path generateDotFormatGraph(CommandContext &CC) {
  auto FilePath = generateTxtFormatGraph(CC.OM);
  std::ifstream TxtFile{FilePath};
  auto Edges = getGraphEdges(TxtFile);
  GraphType G(Edges.cbegin(), Edges.cend());
  if (CC.Com != coms::Cfg && CC.OM[opts::Arg].empty())
    fs::remove(FilePath);
  std::ofstream DotFile{FilePath.replace_extension(".dot")};
  G.dumpInDotFormat(DotFile, CC.OM[opts::NodeShape], CC.OM[opts::NodeColor],
                    CC.OM[opts::EdgeShape], CC.OM[opts::EdgeColor],
                    CC.OM[opts::GraphName]);
  DotFile.close();
  return FilePath;
}

template <DotGraphTipe GraphType>
void generatePngFormatGraph(CommandContext &CC) {
  auto DotFilePath = generateDotFormatGraph<GraphType>(CC);
  dumpInPngFormat(DotFilePath);
  if (CC.Com != coms::Cfg && CC.Com != coms::DomTree && CC.Com != coms::JoinGraph,
      CC.Com != coms::DomFrontier)
    fs::remove(DotFilePath);
  std::system(
      formatPrint("display {}", DotFilePath.replace_extension(".png").string())
          .c_str());
}

template <DotGraphTipe GraphType>
void generateFullExtensionGraph(CommandContext &CC) {
  generatePngFormatGraph<GraphType>(CC);
}

bool checkCLArgsOnValidity(std::string_view Command) {
  if (!ComCodesMap.contains(Command))
    InputErrors.push_back(formatPrint(
        "Input error: {} is not available command. Try -help", Command));

  if (const auto &Path = OptsMap[opts::Path]; !fs::exists(Path))
    InputErrors.push_back(formatPrint("Input error: {} is invalid path", Path));

  auto CheckIntArgOption = [&](std::string_view Option) {
    try {
      int Arg = std::stoi(OptsMap[Option]);
      if (Arg <= 0) {
        InputErrors.push_back(formatPrint(
            "Input error: {}=: invalid argument: {}, must be positive", Option,
            Arg));
      } else {
        return Arg;
      }
    } catch (...) {
      InputErrors.push_back(
          formatPrint("Input error: {}=: invalid argument", Option));
    }
    return 0;
  };

  if (int NumNodes = CheckIntArgOption(opts::NumNodes),
      NumEdges = CheckIntArgOption(opts::NumEdges);
      NumNodes && NumEdges && NumNodes <= NumEdges) {
    InputErrors.push_back(
        formatPrint("Input error: invalid set of arguments: number of nodes "
                    "must be greater than number of edges"));
  }

  return InputErrors.empty();
}

} // namespace

int main(int args, char **argv) {
  if (args < 2) {
    std::cerr << "input error: expected command. Try run with -h" << std::endl;
    return ErrorInputCode;
  }

  std::vector<std::string> OptionSet(std::next(argv), argv + args);
  for (auto &OptStr : OptionSet | std::views::drop(1)) {
    auto Delimetr = OptStr.find_first_of('=');
    auto Opt = OptStr.substr(0, Delimetr);
    if (Delimetr == OptStr.npos || !OptsMap.contains(Opt))
      InputErrors.push_back(
          formatPrint("Input error: invalid option: {}", std::move(Opt)));
    else
      OptsMap[Opt] = std::move(OptStr.substr(++Delimetr));
  }

  if (!checkCLArgsOnValidity(OptionSet.front())) {
    std::ranges::copy(InputErrors,
                      std::ostream_iterator<std::string>(std::cerr, "\n"));
    return ErrorInputCode;
  }

  CommandContext CC{OptsMap, OptionSet.front()};
  switch (getComCode(CC.Com)) {
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
  case ComCodes::JoinGraph:
    generateFullExtensionGraph<DJGT>(CC);
    break;
  case ComCodes::JoinGraphDot:
    generateDotFormatGraph<DJGT>(CC);
    break;
  case ComCodes::JoinGraphPng:
    generatePngFormatGraph<DJGT>(CC);
    break;
  case ComCodes::DomFrontier:
    generateFullExtensionGraph<DFT>(CC);
    break;
  case ComCodes::DomFrontierDot:
    generateDotFormatGraph<DFT>(CC);
    break;
  case ComCodes::DomFrontierPng:
    generatePngFormatGraph<DFT>(CC);
    break;
  default:
    break;
  }
}
