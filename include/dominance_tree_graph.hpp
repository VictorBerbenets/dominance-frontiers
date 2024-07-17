#pragma once

#include <algorithm>
#include <cassert>
#include <concepts>
#include <iterator>
#include <ranges>
#include <map>
#include <queue>
#include <set>

#include "directed_graph.hpp"

namespace graphs {

template <typename T> class DomTreeGraph : public DirectedGraph<T> {
  using DirGraphType = DirectedGraph<T>;
  using DirGraphType::Nodes;
  using typename DirectedGraph<T>::value_type;
  using typename DirGraphType::EdgeType;
  using typename DirGraphType::NodeTypePtr;

  using size_type = std::size_t;
  using DomTable = std::map<NodeTypePtr, std::set<NodeTypePtr>>;

public:
  template <std::forward_iterator ForwIt>
    requires requires(ForwIt It) {
      { *It } -> std::convertible_to<EdgeType>;
    }
  DomTreeGraph(ForwIt FBegin, ForwIt FEnd) : DirGraphType(FBegin, FEnd) {
    std::map<NodeTypePtr, std::vector<NodeTypePtr>> ParentChildsMap;

    for (auto DomTbl = determineDominators();
         auto &[NodePtr, DomSet] : DomTbl) {
      DomSet.erase(NodePtr);
      if (DomSet.empty()) {
        continue;
      } else if (DomSet.size() == 1) {
        auto *Top = *DomSet.begin();
        ParentChildsMap[Top].push_back(NodePtr);
      } else {
        auto *Closest = getClosest(DomSet, NodePtr);
        assert(Closest);
        ParentChildsMap[Closest].push_back(NodePtr);
      }
    }
    // Clean previous graph
    std::ranges::for_each(
        Nodes, [](auto &UniquePtr) { UniquePtr.get()->clearThreads(); });
    // Create tree threads
    for (auto &[ParentPtr, Children] : ParentChildsMap) {
      for (auto *ChildPtr : Children) {
        ParentPtr->addSuccessor(ChildPtr);
      }
    }
  }

private:
  /*
   * Dom(n) = n \/ ( /\ Dom(m)), where m is a set of predecessors of the n
   */
  DomTable determineDominators() const {
    if (Nodes.empty())
      return {};

    DomTable DomTbl;
    std::set<NodeTypePtr> Set;
    std::transform(Nodes.begin(), Nodes.end(), std::inserter(Set, Set.end()),
                   [](auto &UniquePtr) { return UniquePtr.get(); });
    std::transform(Nodes.begin(), Nodes.end(),
                   std::inserter(DomTbl, DomTbl.end()),
                   [&Set](auto &UniquePtr) {
                     return std::make_pair(UniquePtr.get(), Set);
                   });

    auto FrontPtr = Nodes.front().get();
    DomTbl[FrontPtr].clear();
    DomTbl[FrontPtr].insert(FrontPtr);

    bool Changed = true;
    while (Changed) {
      Changed = false;
      for (auto &UniquePtr : Nodes) {
        auto NodePtr = UniquePtr.get();
        auto TmpSet = DomTbl[NodePtr];
        for (auto Pred : NodePtr->getPredecessors()) {
          auto &CompSet = DomTbl[Pred];
          std::set<NodeTypePtr> SaveComp;
          std::set_intersection(CompSet.begin(), CompSet.end(), TmpSet.begin(),
                                TmpSet.end(),
                                std::inserter(SaveComp, SaveComp.end()));
          TmpSet = std::move(SaveComp);
        }
        TmpSet.insert(NodePtr);
        Changed |= DomTbl[NodePtr] != TmpSet;
        if (DomTbl[NodePtr] != TmpSet) {
          DomTbl[NodePtr] = std::move(TmpSet);
        }
      }
    }

    return DomTbl;
  }
  // need to make faster
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
