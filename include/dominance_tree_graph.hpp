#pragma once

#include <algorithm>
#include <concepts>
#include <iterator>
#include <queue>
#include <ranges>
#include <set>

#include "directed_graph.hpp"

namespace graphs {

template <typename T> class DomTreeGraph : public DirectedGraph<T> {
protected:
  using DGT = DirectedGraph<T>;
  using DGT::Nodes;
  using typename DGT::NodeTypePtr;
  using EdgePtrType = std::pair<NodeTypePtr, NodeTypePtr>;

public:
  using DTT = DomTreeGraph<T>;

  template <InputEdgeIter EdgeIt>
  DomTreeGraph(EdgeIt FBegin, EdgeIt FEnd) : DGT(FBegin, FEnd) {
    auto DomTree = getDominatorsTree();
    // Clean previous graph
    DGT::clearGraphThreads();
    // Create tree threads
    for (auto &[ParentPtr, Children] : DomTree) {
      for (auto *ChildPtr : Children) {
        ParentPtr->addSuccessor(ChildPtr);
      }
    }
  }

private:
  std::map<NodeTypePtr, std::vector<NodeTypePtr>> getDominatorsTree() const {
    std::map<NodeTypePtr, std::vector<NodeTypePtr>> DomTree;

    for (auto DomTbl = DGT::determineDominators();
         auto &[NodePtr, DomSet] : DomTbl) {
      DomSet.erase(NodePtr);
      if (DomSet.empty()) {
        continue;
      } else if (DomSet.size() == 1) {
        auto *Top = *DomSet.begin();
        DomTree[Top].push_back(NodePtr);
      } else {
        auto *Closest = getClosest(DomSet, NodePtr);
        assert(Closest);
        DomTree[Closest].push_back(NodePtr);
      }
    }

    return DomTree;
  }

private:
  NodeTypePtr getClosest(const std::set<NodeTypePtr> &DomSet,
                         NodeTypePtr NodePtr) const {
    std::queue<NodeTypePtr> BreadthLineNodes;
    BreadthLineNodes.push(NodePtr);
    while (!BreadthLineNodes.empty()) {
      auto *CurrNodePtr = BreadthLineNodes.front();
      BreadthLineNodes.pop();
      for (auto *Pred : CurrNodePtr->getPredecessors()) {
        if (DomSet.contains(Pred))
          return Pred;
        BreadthLineNodes.push(Pred);
      }
    }

    return nullptr;
  }
};

} // namespace graphs
