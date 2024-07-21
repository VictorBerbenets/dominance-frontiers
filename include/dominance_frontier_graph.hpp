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

template <typename T> class DomJoinGraph : private DomTreeGraph<T> {
protected:
  using DTG = DomTreeGraph<T>;
  using DTG::Nodes;
  using typename DTG::NodeTypePtr;
  using EdgePtrType = std::pair<NodeTypePtr, NodeTypePtr>;

public:
  using DJGT = DomJoinGraph<T>;

  template <ForwEdgeIter FIter>
  DomJoinGraph(FIter Begin, FIter End) : DTG(Begin, End) {
    std::unordered_map<std::string, NodeTypePtr> NodeMap;
    std::transform(Nodes.cbegin(), Nodes.cend(),
                   std::inserter(NodeMap, NodeMap.end()),
                   [](const auto &UnPtr) {
                     return std::make_pair(UnPtr.get()->getName(), UnPtr.get());
                   });
    auto Edges = getEdges();
    std::vector<EdgeType> JoinStringEdges;
    std::set_difference(Begin, End, Edges.begin(), Edges.end(),
                        std::back_inserter(JoinStringEdges));
    std::transform(JoinStringEdges.begin(), JoinStringEdges.end(),
                   std::back_inserter(JoinEdges), [&NodeMap](const auto &Edge) {
                     auto &[V1, V2] = Edge;
                     return std::make_pair(NodeMap[V1], NodeMap[V2]);
                   });
  }

  void dumpInDotFormat(std::ofstream &DotDump, std::string_view GraphName,
                       std::string_view NodeShape, std::string_view NodeColor,
                       std::string_view EdgeShape,
                       std::string_view EdgeColor) const override {
    DTG::dumpInDotFormatBaseImpl(DotDump, GraphName, NodeShape, NodeColor,
                                 EdgeShape, EdgeColor);
    std::transform(JoinEdges.cbegin(), JoinEdges.cend(),
                   std::ostream_iterator<std::string>(DotDump),
                   [](const auto &Edge) {
                     return utils::formatPrint("{} -> {} [style = dotted];\n",
                                               Edge.first->getName(),
                                               Edge.second->getName());
                   });

    DotDump << "}\n";
  }

private:
  std::vector<EdgeType> getEdges() const {
    if (Nodes.empty())
      return {};

    std::vector<EdgeType> Edges;
    for (const auto &UnNodePtr : Nodes) {
      for (auto ParentPtr = UnNodePtr.get();
           const auto *NodePtr : ParentPtr->getSuccessors()) {
        Edges.emplace_back(ParentPtr->getName(), NodePtr->getName());
      }
    }
    return Edges;
  }

protected:
  std::vector<EdgePtrType> JoinEdges;
};

template <typename T> class DomFrontierGraph final : private DomJoinGraph<T> {
  using DJGT = DomJoinGraph<T>;
  using typename DJGT::DTG::NodeTypePtr;

public:
  template <ForwEdgeIter FIter>
  DomFrontierGraph(FIter Begin, FIter End) : DJGT(Begin, End) {
    std::set<NodeTypePtr> ImmediateDomSet;
  }

  void dumpInDotFormat(std::ofstream &DotDump, std::string_view GraphName,
                       std::string_view NodeShape, std::string_view NodeColor,
                       std::string_view EdgeShape,
                       std::string_view EdgeColor) const override {}
};

} // namespace graphs
