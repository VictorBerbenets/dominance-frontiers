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
    auto DomTree = DGT::getDominatorsTree();
    // Clean previous graph
    DGT::clearGraphThreads();
    // Create tree threads
    for (auto &[ParentPtr, Children] : DomTree) {
      for (auto *ChildPtr : Children) {
        ParentPtr->addSuccessor(ChildPtr);
      }
    }
  }
};

} // namespace graphs
