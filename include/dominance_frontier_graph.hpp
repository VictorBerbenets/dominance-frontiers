#pragma once

#include <algorithm>
#include <fstream>
#include <iterator>
#include <ranges>
#include <set>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "dominance_tree_graph.hpp"

namespace graphs {

namespace rgs = std::ranges;

template <typename T>
concept ForwEdgeIter = InputEdgeIter<T> && std::forward_iterator<T>;

template <typename T> class DomJoinGraph : protected DomTreeGraph<T> {
protected:
  using DTG = DomTreeGraph<T>;
  using DTG::Nodes;
  using typename DTG::DGT;
  using typename DTG::NodeTypePtr;
  using EdgePtrType = std::pair<NodeTypePtr, NodeTypePtr>;

  static constexpr std::string_view DefGraphName = "Dominance Join Graph";

public:
  using DJGT = DomJoinGraph<T>;

  template <ForwEdgeIter FIter>
  DomJoinGraph(FIter Begin, FIter End) : DTG(Begin, End) {
    std::unordered_map<std::string, NodeTypePtr> NodeMap;
    rgs::transform(Nodes, std::inserter(NodeMap, NodeMap.end()),
                   [](const auto &UnPtr) {
                     return std::make_pair(UnPtr.get()->getName(), UnPtr.get());
                   });

    auto Edges = getEdges();
    std::set<EdgeType> CompSet(Begin, End);
    std::vector<EdgeType> JoinStringEdges;
    rgs::set_difference(CompSet, Edges, std::back_inserter(JoinStringEdges));

    rgs::transform(JoinStringEdges, std::back_inserter(JoinEdges),
                   [&NodeMap](const auto &Edge) {
                     auto &[V1, V2] = Edge;
                     return std::make_pair(NodeMap[V1], NodeMap[V2]);
                   });
  }

  void dumpInDotFormat(std::ofstream &DotDump, std::string_view GraphName,
                       std::string_view NodeShape, std::string_view NodeColor,
                       std::string_view EdgeShape,
                       std::string_view EdgeColor) const override {
    DGT::dumpInDotFormatBaseImpl(DotDump, GraphName, NodeShape, NodeColor,
                                 EdgeShape, EdgeColor);
    rgs::transform(JoinEdges, std::ostream_iterator<std::string>(DotDump),
                   [](const auto &Edge) {
                     return utils::formatPrint("{} -> {} [style = dotted];\n",
                                               Edge.first->getName(),
                                               Edge.second->getName());
                   });

    DotDump << "}\n";
  }

private:
  std::set<EdgeType> getEdges() const {
    if (Nodes.empty())
      return {};

    std::set<EdgeType> Edges;
    for (const auto &UnNodePtr : Nodes)
      for (auto ParentPtr = UnNodePtr.get();
           const auto *NodePtr : ParentPtr->getSuccessors())
        Edges.emplace(ParentPtr->getName(), NodePtr->getName());

    return Edges;
  }

protected:
  std::vector<EdgePtrType> JoinEdges;
};

template <typename T> class DomFrontierGraph final : protected DomJoinGraph<T> {
  using DJGT = DomJoinGraph<T>;
  using DJGT::JoinEdges;
  using DJGT::Nodes;
  using typename DJGT::DTG::DGT;
  using typename DJGT::DTG::NodeTypePtr;

  static constexpr std::string_view DefGraphName = "Dominance Frontier";

public:
  template <ForwEdgeIter FIter>
  DomFrontierGraph(FIter Begin, FIter End) : DJGT(Begin, End) {
    std::map<NodeTypePtr, std::set<NodeTypePtr>> DomFront;
    auto IDom = getImmediateDominatorSet();
    // making 'join' links
    rgs::for_each(JoinEdges, [](const auto &EdgePtr) {
      auto [V1, V2] = EdgePtr;
      V1->addSuccessor(V2);
    });

    for (const auto &UniqPtr : Nodes)
      for (auto ParentPtr = UniqPtr.get();
           auto *Pred : ParentPtr->getPredecessors())
        for (auto TmpPtr = Pred; TmpPtr != IDom[ParentPtr];
             TmpPtr = IDom[TmpPtr])
          DomFront[TmpPtr].insert(ParentPtr);

    DGT::clearGraphThreads();

    for (auto &[Parent, Children] : DomFront) {
      for (auto *Child : Children)
        Parent->addSuccessor(Child);
    }
  }

  void dumpInDotFormat(std::ofstream &DotDump, std::string_view GraphName,
                       std::string_view NodeShape, std::string_view NodeColor,
                       std::string_view EdgeShape,
                       std::string_view EdgeColor) const override {
    DGT::dumpInDotFormatBaseImpl(DotDump, GraphName, NodeShape, NodeColor,
                                 EdgeShape, EdgeColor);
    for (auto &UnPtr : Nodes)
      if (auto NodePtr = UnPtr.get(); NodePtr->getSuccessorsCount() == 0)
        DotDump << utils::formatPrint("{} -> {};\n", NodePtr->getName(),
                                      NodePtr->getName());

    DotDump << "}\n";
  }

private:
  std::unordered_map<NodeTypePtr, NodeTypePtr> getImmediateDominatorSet() {
    std::unordered_map<NodeTypePtr, NodeTypePtr> ImmediateDomSet;
    for (const auto &UniqPtr : Nodes)
      for (auto *Child = UniqPtr.get(); auto *Parent : Child->getPredecessors())
        ImmediateDomSet.emplace(Child, Parent);

    return ImmediateDomSet;
  }
};

} // namespace graphs
