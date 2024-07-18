#pragma once

#include <algorithm>
#include <fstream>
#include <iterator>
#include <string_view>
#include <vector>

#include "directed_graph.hpp"
#include "dominance_tree_graph.hpp"

namespace graphs {

template <typename T>
concept ForwEdgeIter = InputEdgeIter<T> && std::forward_iterator<T>;

namespace rgs = std::ranges;

template <typename T> class DomJoinGraph final : private DomTreeGraph<T> {
  using DTG = DomTreeGraph<T>;
  using DTG::Nodes;

public:
  template <ForwEdgeIter FIter>
  DomJoinGraph(FIter Begin, FIter End) : DTG(Begin, End) {
    auto TreeEdges = DTG::getEdges();
    std::set_difference(Begin, End, TreeEdges.cbegin(), TreeEdges.cend(),
                        std::back_inserter(JoinEdges));
  }

  void dumpInDotFormat(std::ofstream &DotDump, std::string_view GraphName,
                       std::string_view NodeShape, std::string_view NodeColor,
                       std::string_view EdgeShape,
                       std::string_view EdgeColor) const override {
    DTG::dumpInDotFormatBaseImpl(DotDump, GraphName, NodeShape, NodeColor,
                                 EdgeShape, EdgeColor);
    for (const auto &[V1, V2] : JoinEdges) {
      DotDump << utils::formatPrint("{} -> {} [style = dotted];\n", V1, V2);
    }

    DotDump << "}\n";
  }

private:
  std::vector<EdgeType> JoinEdges;
};

template <typename T> class DomFrontGraph final : public DirectedGraph<T> {};

} // namespace graphs
