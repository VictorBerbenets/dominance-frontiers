#pragma once

#include <algorithm>
#include <fstream>
#include <iterator>
#include <string_view>
#include <vector>

#include "dominance_tree_graph.hpp"

namespace graphs {

template <typename T>
concept ForwEdgeIter = InputEdgeIter<T> && std::forward_iterator<T>;

namespace rgs = std::ranges;

template <typename T> class DomJoinGraph final : private DomTreeGraph<T> {
protected:
  using DTG = DomTreeGraph<T>;
  using DTG::Nodes;

public:
  using DJGT = DomJoinGraph<T>;

  template <ForwEdgeIter FIter>
  DomJoinGraph(FIter Begin, FIter End) : DTG(Begin, End) {
    rgs::set_difference(rgs::subrange(Begin, End), DTG::getEdges(),
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

protected:
  std::vector<EdgeType> JoinEdges;
};

template <typename T> class DomFrontGraph final : private DomJoinGraph<T> {
  using DJGT = DomJoinGraph<T>;
  using typename DJGT::DTG::NodeTypePtr;

public:
  template <ForwEdgeIter FIter>
  DomFrontGraph(FIter Begin, FIter End) : DJGT(Begin, End) {
    std::set<NodeTypePtr> ImmediateDomSet;
  }
};

} // namespace graphs
