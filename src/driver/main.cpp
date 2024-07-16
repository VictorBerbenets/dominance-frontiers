#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <string>
#include <string_view>
#include <unordered_map>

#include "directed_graph.hpp"
#include "dominance_tree_graph.hpp"
#include "graph_generator.hpp"

namespace {
  
using value_type = int;
using DGT = graphs::DirectedGraph<value_type>;
using DTT = graphs::DomTree<value_type>;
using DGBT = graphs::DirGraphBuilder;
using OptIter = typename std::vector<std::string>::iterator;

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

enum class OpCodes : char {
  Path,
  NumNodes,
  NumEdges,
  NodeColor,
  EdgeColor,
  NodeShape,
  EdgeShape,
  Name
};

template <typename Code, typename MapType>
  requires requires(const MapType &M, std::string_view Com) {
    {M.find(Com)};
  } 
Code findCode(MapType M, std::string_view FindCode) {
  auto FindIt = M.find(FindCode);
  if (FindIt == M.end())
    throw std::runtime_error{
        std::string(FindCode).append(" is not available command. Try -help")};
  return FindIt->second;

}

ComCodes getComCode(std::string_view Command) {

  static std::unordered_map<std::string_view, ComCodes> ComCodesMap {
      {"-h", ComCodes::Help},
      {"-help", ComCodes::Help},
      {"-gen=cfg-dot", ComCodes::CfgDot},
      {"-gen=cfg-png", ComCodes::CfgPng},
      {"-gen=cfg", ComCodes::Cfg},
      {"-gen=dom-tree", ComCodes::DomTree},
      {"-gen=cfg-txt", ComCodes::CfgTxt},
      {"-gen=dom-tree-dot", ComCodes::DomTreeDot},
      {"-gen=dom-tree-png", ComCodes::DomTreePng},

  return findCode<ComCodes>(ComCodesMap, Command); 
}

OpCodes getOpCode(std::string_view Opt) {
  static std::unordered_map<std::string_view, OpCodes> OpCodesMap{
      {"--path=", OpCodes::Path},
      {"--num-nodes=", OpCodes::NumNodes},
      {"--num-edges=", OpCodes::NumEdges},
      {"--node-color=", OpCodes::NodeColor},
      {"--edge-color=", OpCodes::EdgeColor},
      {"--node-shape=", OpCodes::NodeShape},
      {"--edge-shape=", OpCodes::EdgeShape},
      {"--file-name=", OpCodes::Name},
      {"--node-name=", OpCodes::Name} };

  return findCode<OpCodes>(OpCodesMap, Opt);
}


void printHelp(std::ostream &Os = std::cout) {
  Os << std::string(47, '-') << "help" << std::string(49, '-') << std::endl;
  Os << "|-"
     << "To generate CFG in choosen format use next commands:" << std::endl;
  Os << "|\t" << "-gen=cfg-dot\n|\t-gen=cfg-pnh\n|\t-gen=cfg-txt\n|\t-gen=cfg"
     << std::endl;
  Os << "|-"
     << "To generate dominance tree in choosen format use next commands:"
     << std::endl;
  Os << "|\t"
     << "-gen=dom-tree-dot\n|\t-gen=dom-tree-png\n|\t-gen=dom-tree"
     << std::endl;
  Os << "|-"
     << "Note: commands -gen=cfg and -gen=dom-tree generate all graph formats."
     << std::endl;
  Os << "|-" << "Required option:" << std::endl;
  Os << "|\t" << "--path=<>       - path to create files." << std::endl;
  Os << "|-" << "Additional options:" << std::endl;
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
  Os << "|\t" << "--file-name=<> - set name for generated file(s). graph setted as default" << std::endl;
  Os << "|\t" << "--node-name=<> - set name for nodes. BB setted as default" << std::endl;
  Os << "|-"
     << "Note: you can use RGB format for color option (e.g. "
        "--node-color=#ffffff)."
     << std::endl;
  Os << std::string(100, '-') << std::endl;
}

template <typename Map>
OptIter generateFullExtensionGraph(const Map &);

template <typename Map>
OptIter generateDotFormatGraph(const Map &);

template <typename Map>
OptIter generatePngFormatGraph(const Map &);

OptIter generateTxtFormatGraph(const Map &OptMap) { 
  for ()
  DGBT DGB();
}

template<>
OptIter generateFullExtensionGraph<DGT>(OptIter Curr, OptIter End) {

}

} // namespace

int main(int args, char **argv) {
  if (args < 2) {
    throw std::runtime_error{"input error: expected command. Try run with -h\n"};
  }

  static constexpr std::string_view DefFileName = "graph";
  const std::unordered_map<std::string, std::string> DefMap 

  auto OptionSet = getOptionSet(--args, ++argv);
  for (auto OptIt = OptionSet.begin(), OptEndIt = OptionSet.end();
       OptIt != OptEndIt; ) {
    std::unordered_map<std::string, std::string> 
    for () {

    }

    switch (getComCode(*OptIt)) {
      case ComCodes::Help:
        printHelp();
        break;
      case ComCodes::Cfg:
        OptIt = generateFullExtensionGraph<DGT>();
        break;
      case ComCodes::CfgTxt:
        OptIt = generateTxtFormatGraph<DGT>();
        break;
      case ComCodes::CfgDot:
        OptIt = generateDotFormatGraph<DGT>();
        break;
      case ComCodes::CfgPng:
        OptIt = generatePngFormatGraph<DGT>();
        break;
      case ComCodes::DomTree:
        OptIt = generateFullExtensionGraph<DTT>();
        break;
      case ComCodes::DomTreeDot:
        OptIt = generateDotFormatGraph<DTT>();
        break;
      case ComCodes::DomTreePng:
        OptIt = generatePngFormatGraph<DTT>();
        break;
      default:
        throw std::runtime_error{
          std::string(*OptIt).append(" is not available command. Try -help")};
    }
  }
}
