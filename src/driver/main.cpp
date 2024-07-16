#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <filesystem>
#include <stdexcept>
#include <vector>
#include <string>
#include <string_view>
#include <unordered_map>
#include <ranges>

#include "directed_graph.hpp"
#include "dominance_tree_graph.hpp"
#include "graph_generator.hpp"

namespace {

namespace fs = std::filesystem; 

using namespace std::literals; 
using namespace string_literals; 
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
  static constexpr std::string_view Path = "--path";
  static constexpr std::string_view NumNodes = "--num-nodes";
  static constexpr std::string_view NumEdges = "--num-edges";
  static constexpr std::string_view NodeColor = "--node-color";
  static constexpr std::string_view EdgeColor = "--edge-color";
  static constexpr std::string_view NodeShape = "--node-shape";
  static constexpr std::string_view EdgeShape = "--edge-shape";
  static constexpr std::string_view FileName = "--file-name";
  static constexpr std::string_view NodeName = "--node-name";
}; // namespace opts

namespace coms {
  static constexpr std::string_view H = "-h";
  static constexpr std::string_view Help = "-help";
  static constexpr std::string_view Cfg = "-g=cfg";
  static constexpr std::string_view CfgTxt = "-g=cfg-txt";
  static constexpr std::string_view CfgDot = "-g=cfg-dot";
  static constexpr std::string_view CfgPng = "-g=cfg-png";
  static constexpr std::string_view DomTree = "-g=dom-tree";
  static constexpr std::string_view DomTreeDot = "-g=dom-dot";
  static constexpr std::string_view DomTreePng = "-g=dom-png";
}; // namespace opts

auto getOptionSet(int args, char **argv) {
  std::vector<std::string> OptionsSet(argv, argv + args);
  return OptionsSet;
}

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
  static std::unordered_map<std::string_view, ComCodes> ComCodesMap {
      {coms::H, ComCodes::Help},
      {coms::Help, ComCodes::Help},
      {coms::CfgDot, ComCodes::CfgDot},
      {coms::CfgPng, ComCodes::CfgPng},
      {coms::Cfg, ComCodes::Cfg},
      {coms::DomTree, ComCodes::DomTree},
      {coms::CfgTxt, ComCodes::CfgTxt},
      {coms::DomTreeDot, ComCodes::DomTreeDot},
      {coms::DomTreePng, ComCodes::DomTreePng}, };
  
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
  Os << "|\t"
     << "-g=dom-tree-dot\n|\t-g=dom-tree-png\n|\t-g=dom-tree"
     << std::endl;
  Os << "|-"
     << "Note: commands -g=cfg and -g=dom-tree generate all graph formats."
     << std::endl;
  Os << "|-" << "Options:" << std::endl;
  Os << "|\t" << "--path=<>       - path to create files." << std::endl;
  Os << "|\t" << "--num-nodes=<>  - set number of nodes." << std::endl;
  Os << "|\t" << "--num-edges=<>  - set the limit on the number of node edges."
     << std::endl;
  Os << "|\t" << "--node-color=<> - set node color (lightblue is default)."
     << std::endl;
  Os << "|\t" << "--edge-color=<> - set edge color (red is default)."
     << std::endl;
  Os << "|\t"
     << "--node-shape=<> - set node shape. circle is default. Check graphviz.org "
        "for more."
     << std::endl;
  Os << "|\t"
     << "--edge-shape=<> - set edge shape. vee is default. Check graphviz.org for "
        "more."
     << std::endl;
  Os << "|\t" << "--file-name=<>  - set name for generated file(s). graph setted as default." << std::endl;
  Os << "|\t" << "--node-name=<>  - set name for nodes. BB setted as default." << std::endl;
  Os << "|-"
     << "Note: you can use RGB format for color option (e.g. "
        "--node-color=#ffffff)."
     << std::endl;
  Os << std::string(100, '-') << std::endl;
}

int getOptInt(std::string_view Opt, std::string_view Arg) {
  if (int Num = std::stoi(std::string(Arg)); Num < 0)
    throw std::runtime_error {std::string(Opt).append(": invalid argument "s.append(Arg))};
  else
    return Num;
}

fs::path getPath(std::string_view PathStr) { 
  if (fs::path Path {PathStr}; !fs::exists(Path))
    throw std::runtime_error {Path.string().append(" is invalid path")};
  else
    return Path;
}

template <typename T>
void generateFullExtensionGraph(OptMap &);

template <typename T>
void generateDotFormatGraph(CommandContext &CC);

template <typename T>
void generatePngFormatGraph(CommandContext &CC);

fs::path generateTxtFormatGraph(OptMap &OM) {
  auto FilePath = getPath(OM[opts::Path]).append(OM[opts::FileName]).replace_extension(".txt");
  std::ofstream TxtFile {FilePath}; 
  DGBT::generateGraph(TxtFile,
                      getOptInt(opts::NumNodes, OM[opts::NumNodes]),
                      getOptInt(opts::NumEdges, OM[opts::NumEdges]),
                      OM[opts::NodeName]);
  return FilePath;
}

template <>
void generateDotFormatGraph<DGT>(CommandContext &CC) {
  auto TxtFilePath = generateTxtFormatGraph(CC.OM);
  std::ifstream TxtFile{TxtFilePath};
  auto Edges = getGraphEdges(TxtFile);
  DGT DG(Edges.cbegin(), Edges.cend());
  std::ifstream DotFile{TxtFilePath.replace_extension(".dot")};
  DG.dumpInDotFormat(DotFile);

  if (CC.Com != coms::Cfg && CC.Com != coms::CfgPng) {
    fs::remove(TxtFile);  
  }
}

template<>
void generatePngFormateGraph<DGT>(CommandContext &CC) {
  
}

template<>
void generateFullExtensionGraph<DGT>(OptMap &OM) {

}

template <std::input_iterator Iter>
void printInvalidOptions(Iter Begin, Iter end, std::ostream &Os = std::cout) {
  Os << "Error: invalid option"s + (end == std::next(Begin) ? ":" : "s:") << std::endl;
  std::copy(Begin, end, std::ostream_iterator<std::string>(Os, "\n"));
  Os << "Try run with -h" << std::endl;
}

} // namespace

int main(int args, char **argv) {
  if (args < 2) {
    throw std::runtime_error{"input error: expected command. Try run with -h\n"};
  }

  static constexpr std::string_view DefFileName = "graph";
  OptMap OptsMap {
      {opts::Path, "."},
      {opts::NumNodes, std::to_string(DGBT::DefNodeNum)},
      {opts::NumEdges, std::to_string(DGBT::DefEdgeNum)},
      {opts::NodeColor, std::string(DGT::DefNodeColor)},
      {opts::EdgeColor, std::string(DGT::DefEdgeColor)},
      {opts::NodeShape, std::string(DGT::DefNodeShape)},
      {opts::EdgeShape, std::string(DGT::DefEdgeShape)},
      {opts::FileName, std::string(DefFileName)},
      {opts::NodeName, std::string(DGBT::DefNodeName)} };

  auto OptionSet = getOptionSet(--args, ++argv);
  std::vector<std::string> ErrorOpts;
  for (auto &OptStr : OptionSet | std::views::drop(1)) {
    auto Delimetr = OptStr.find_first_of('=');
    auto Opt = OptStr.substr(0, Delimetr);
    std::cout << "OPT = \"" << Opt << "\"" << std::endl;
    if (Delimetr == OptStr.npos || !OptsMap.contains(Opt))
      ErrorOpts.push_back(std::move(Opt));
    else
      OptsMap[Opt] = std::move(OptStr.substr(++Delimetr));
    std::cout << "ARG = " << OptsMap[Opt] << std::endl;
  }
  
  if (!ErrorOpts.empty()) {
    printInvalidOptions(ErrorOpts.cbegin(), ErrorOpts.cend());
  }

  switch (getComCode(OptionSet.front())) {
    case ComCodes::Help:
      printHelp();
      break;
#if 0
    case ComCodes::Cfg:
      generateFullExtensionGraph<DGT>();
      break;
#endif
    case ComCodes::CfgTxt:
      generateTxtFormatGraph(OptsMap);
      break;
#if 0
    case ComCodes::CfgDot:
      generateDotFormatGraph<DGT>();
      break;
    case ComCodes::CfgPng:
      generatePngFormatGraph<DGT>();
      break;
    case ComCodes::DomTree:
      generateFullExtensionGraph<DTT>();
      break;
    case ComCodes::DomTreeDot:
      generateDotFormatGraph<DTT>();
      break;
    case ComCodes::DomTreePng:
      generatePngFormatGraph<DTT>();
      break;
#endif
    default:
      throw std::runtime_error{
        std::string(OptionSet.front()).append(" is not available command. Try -help")};
  }
}
