#pragma once

#include <algorithm>
#include <concepts>
#include <iterator>
#include <map>
#include <set>
#include <unordered_map>

#include "directed_graph.hpp"

namespace graphs {

template <typename T> class DomTreeGraph : public DirectedGraph<T> {
  using DirGraphType = DirectedGraph<T>;
  using typename DirectedGraph<T>::value_type;
  using DirGraphType::Nodes;
  using typename DirGraphType::EdgeType;
  using typename DirGraphType::NodeTypePtr;

  using size_type = std::size_t;
  using DomTable = std::map<std::string, std::set<NodeTypePtr>>;

public:
  template <std::forward_iterator ForwIt>
    requires requires(ForwIt It) {
      { *It } -> std::convertible_to<EdgeType>;
    }
  DomTreeGraph(ForwIt FBegin, ForwIt FEnd) : DirGraphType(FBegin, FEnd) {
    auto DomTbl = determineDominators();
    // TODO 
  }

  /*
   * Dom(n) = n \/ ( /\ Dom(m)), where m is a set of predecessors of the n
   */
  DomTable determineDominators() const {
    DomTable DomTbl;
    std::set<NodeTypePtr> Set;
    std::transform(Nodes.begin(), Nodes.end(), std::inserter(Set, Set.end()),
                   [](auto &UniquePtr) { return UniquePtr.get(); });
    std::transform(Nodes.begin(), Nodes.end(),
                   std::inserter(DomTbl, DomTbl.end()),
                   [&Set](auto &UniquePtr) {
                     return std::make_pair(UniquePtr.get()->getName(), Set);
                   });

    auto FrontPtr = Nodes.front().get();
    DomTbl[FrontPtr->getName()].clear();
    DomTbl[FrontPtr->getName()].insert(FrontPtr);

    bool Changed = true;
    while (Changed) {
      Changed = false;
      for (auto &UniquePtr : Nodes) {
        auto NodePtr = UniquePtr.get();
        auto TmpSet = DomTbl[NodePtr->getName()];
        for (auto Pred : NodePtr->getPredecessors()) {
          auto &CompSet = DomTbl[Pred->getName()];
          std::set<NodeTypePtr> SaveComp;
          std::set_intersection(CompSet.begin(), CompSet.end(), TmpSet.begin(),
                                TmpSet.end(),
                                std::inserter(SaveComp, SaveComp.end()));
          TmpSet = std::move(SaveComp);
        }
        TmpSet.insert(NodePtr);
        Changed |= DomTbl[NodePtr->getName()] != TmpSet;
        if (auto Name = NodePtr->getName(); DomTbl[Name] != TmpSet) {
          DomTbl[Name] = std::move(TmpSet);
        }
      }
    }

    return DomTbl;
  }

private:
  NodeTypePtr Root;
};

} // namespace graphs
