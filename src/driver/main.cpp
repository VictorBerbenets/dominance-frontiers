#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_set>

#include "directed_graph.hpp"
#include "dominance_tree_graph.hpp"

namespace {

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
  std::unordered_set<std::string> OptionsSet;
  std::transform(argv, argv + args, std::inserter(OptionsSet, OptionsSet.end()),
                 [](auto *Str) { return std::string(Str); });

  return OptionsSet;
}

enum class OpCodes : char {
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

OpCodes getOpCode(std::string_view Command) {

  static std::unordered_map<std::string_view, OpCodes> ComOpcodes{
      {"--help", OpCodes::Help},
      {"-gen=cfg-dot", OpCodes::CfgDot},
      {"-gen=cfg-png", OpCodes::CfgPng},
      {"-gen=cfg", OpCodes::Cfg},
      {"-gen=dom-tree", OpCodes::DomTree},
      {"-gen=cfg-txt", OpCodes::CfgTxt},
      {"-gen=dom-tree-dot", OpCodes::DomTreeDot},
      {"-gen=dom-tree-png", OpCodes::DomTreePng},
      {"-gen=dom-tree-txt", OpCodes::DomTreeTxt}};

  auto OpIt = ComOpcodes.find(Command);
  if (OpIt == ComOpcodes.end())
    throw std::runtime_error{
        std::string(Command).append(" is not available command. Try --help")};
  return OpIt->second;
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
     << "-gen=dom-tree-dot\n|\t-gen=dom-tree-png\n|\t-gen=dom-tree-txt\n|\t-"
        "gen=dom-tree"
     << std::endl;
  Os << "|-"
     << "Note: commands -gen=cfg and -gen=dom-tree generate all graph formats."
     << std::endl;
  Os << "|-" << "Required option:" << std::endl;
  Os << "|\t" << "--path=<> - path to create files." << std::endl;
  Os << "|-" << "Additional options:" << std::endl;
  Os << "|\t" << "--ncount=<> - set number of nodes." << std::endl;
  Os << "|\t" << "--nedges=<> - set the limit on the number of node edges."
     << std::endl;
  Os << "|\t" << "--node-color=<> - set node color (lightblue is default)."
     << std::endl;
  Os << "|\t" << "--edge-color=<> - set edge color (red is default)."
     << std::endl;
  Os << "|\t"
     << "--node-shape=<> - set node shape. circle is default. Check graphviz "
        "for more."
     << std::endl;
  Os << "|\t"
     << "--edge-shape=<> - set edge shape. vee is default. Check graphviz for "
        "more."
     << std::endl;
  Os << "|\t" << "--name=<> - set the name for generated file(s)." << std::endl;
  Os << "|-"
     << "Note: you can use RGB format for color option (e.g. "
        "--node-color=#ffffff)."
     << std::endl;
  Os << std::string(100, '-') << std::endl;
}

} // namespace

int main(int args, char **argv) {
  if (args < 2) {
    throw std::runtime_error{"input error: expected file name"};
  }
  auto OptionSet = getOptionSet(--args, ++argv);
  for (auto OptIt = OptionSet.begin(), OptEndIt = OptionSet.end();
       OptIt != OptEndIt; ++OptIt) {
    switch (getOpCode(*OptIt)) {
    case OpCodes::Help:
      printHelp();
      break;
    case OpCodes::Cfg:
    case OpCodes::CfgDot:
    case OpCodes::CfgPng:
    case OpCodes::DomTree:
      break;
    case OpCodes::DomTreeDot:
    case OpCodes::DomTreePng:
    default:
      break;
    }
  }
#if 0
  std::ifstream GraphFile{argv[1]};
  auto Edges = getGraphEdges(GraphFile);
  graphs::DomTreeGraph<int> DmGraph(Edges.cbegin(), Edges.cend());
  std::ofstream DotFormat("dot-graph.dot");
  DmGraph.dumpInDotFormat(DotFormat, "circle", "lightblue", "vee", "red");
  graphs::dumpIntoPng(".", "dot-graph");
#endif
}
