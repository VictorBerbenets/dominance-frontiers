#pragma once

#include <algorithm>
#include <cassert>
#include <concepts>
#include <cstdlib>
#include <filesystem>
#include <iterator>
#include <map>
#include <memory>
#include <queue>
#include <ranges>
#include <set>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include "utils.hpp"

namespace graphs {

namespace rgs = std::ranges;

template <typename T>
  requires std::is_default_constructible_v<T>
class DirectedGraph;

template <typename Data = int>
class DirGraphNode {
  using DirGraphPtr = DirectedGraph<Data> *;

public:
  using size_type = std::size_t;
  using NodePtr = DirGraphNode*;

  DirGraphNode(Data Dat, const std::string &Name = "",
               DirGraphPtr Parent = nullptr)
      : Dat(Dat), Name(Name), Parent(Parent) {}

  virtual ~DirGraphNode() {}

  const Data &getData() const noexcept { return Dat; }
  Data &getData() noexcept { return Dat; }
  
  std::string getName() const { return Name; }

  void addSuccessor(NodePtr Ptr) {
    Successors.push_back(Ptr);
    Ptr->addPredecessor(this);
  }

  void removeSuccessor(NodePtr Ptr) {
    if (auto RmIter = rgs::find(Successors, Ptr); RmIter != Successors.end()) {
      (*RmIter)->removePredecessor(this);
      Successors.erase(RmIter);
    }
  }
  void addPredecessor(NodePtr Ptr) { Predecessors.push_back(Ptr); }
  void removePredecessor(NodePtr Ptr) { std::erase(Predecessors, Ptr); }

  DirGraphPtr getParent() const noexcept { return Parent; }

  auto getSuccessors() const { return rgs::subrange{Successors}; }
  auto getPredecessors() const { return rgs::subrange{Predecessors}; }

  auto getSuccessors() { return rgs::subrange{Successors}; }

  auto getPredecessors() { return rgs::subrange{Predecessors}; }

  NodePtr getPredecessor() {
    if (Predecessors.size() == 1) {
      return Predecessors.front();
    }
    return nullptr;
  }
  
  NodePtr getSuccessor() {
    if (Successors.size() == 1) {
      return &Successors.front();
    }
    return nullptr;
  }

  size_type getPredecessorsCount() const noexcept { return Predecessors.size(); }
  size_type getSuccessorsCount() const noexcept { return Successors.size(); }

  void clearThreads() noexcept {
    Successors.clear();
    Predecessors.clear();
  }

private:
  Data Dat;
  std::string Name;
  DirGraphPtr Parent;
  std::vector<NodePtr> Successors;
  std::vector<NodePtr> Predecessors;
};

using EdgeType = std::pair<std::string, std::string>;

template <typename T>
concept InputEdgeIter = std::input_iterator<T> && requires(T It) {
  { *It } -> std::convertible_to<EdgeType>;
};

template <typename T>
  requires std::is_default_constructible_v<T>
class DirectedGraph {
protected:
  using NodeType = DirGraphNode<T>;
  using NodeTypePtr = NodeType *;
  using StoredNodePtr = std::unique_ptr<NodeType>;

public:
  using value_type = T;
  using DomTable = std::map<NodeTypePtr, std::set<NodeTypePtr>>;
  using DGT = DirectedGraph<value_type>;

  static constexpr std::string_view DefGraphName = "CFG";
  static constexpr std::string_view DefNodeColor = "lightblue";
  static constexpr std::string_view DefNodeShape = "square";
  static constexpr std::string_view DefEdgeColor = "red";
  static constexpr std::string_view DefEdgeShape = "vee";
public:
  template <InputEdgeIter InputIt>
  DirectedGraph(InputIt BeginIt, InputIt EndIt) {
    std::unordered_map<std::string, NodeType *> Vertices;

    for (; BeginIt != EndIt; ++BeginIt) {
      for (auto &V : {BeginIt->first, BeginIt->second}) {
        if (!Vertices.contains(V)) {
          Nodes.push_back(std::make_unique<NodeType>(value_type(), V, this));
          Vertices.emplace(std::move(V), Nodes.back().get());
        }
      }
      Vertices[BeginIt->first]->addSuccessor(Vertices[BeginIt->second]);
    }
  }

  virtual ~DirectedGraph() {}

  virtual void
  dumpInDotFormat(std::ofstream &DotDump,
                  std::string_view GraphName = DefGraphName,
                  std::string_view NodeShape = DefNodeShape,
                  std::string_view NodeColor = DefNodeColor,
                  std::string_view EdgeShape = DefEdgeShape,
                  std::string_view EdgeColor = DefEdgeColor) const {
    dumpInDotFormatBaseImpl(DotDump, GraphName, NodeShape, NodeColor, EdgeShape,
                            EdgeColor);
    DotDump << "}\n";
  }

  // access random graph node ptr
  NodeTypePtr getNodePtr() const noexcept { return Nodes.front().get(); }

  /*
   * Dom(n) = n \/ ( /\ Dom(m)), where m is a set of predecessors of the n
   */
  DomTable determineDominators() const {
    if (Nodes.empty())
      return {};

    DomTable DomTbl;
    std::set<NodeTypePtr> Set;
    rgs::transform(Nodes, std::inserter(Set, Set.end()),
                   [](auto &UniquePtr) { return UniquePtr.get(); });
    rgs::transform(Nodes, std::inserter(DomTbl, DomTbl.end()),
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
          rgs::set_intersection(CompSet, TmpSet,
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

protected:
  void dumpInDotFormatBaseImpl(std::ofstream &DotDump,
                               std::string_view NodeShape,
                               std::string_view NodeColor,
                               std::string_view EdgeShape,
                               std::string_view EdgeColor,
                               std::string_view GraphName) const {

    DotDump << utils::formatPrint(
        "digraph {} {}\n"
        "\tlabel=\"{}\"\n"
        "\tdpi = 100;\n"
        "\tfontname = \"Comic Sans MS\";\n"
        "\tfontsize = 20;\n"
        "\trankdir  = TB;\n"
        "node [shape = {}, style = filled, fillcolor = \"{}\"];\n"
        "edge [color = {}, arrowhead = {}, arrowsize = 1,"
        "penwidth = 1.2];\n",
        GraphName, '{', GraphName, NodeShape, NodeColor, EdgeColor, EdgeShape);

    for (const auto &Ptr : Nodes) {
      auto Name = Ptr.get()->getName();
      for (auto Vertex : Ptr.get()->getSuccessors()) {
        DotDump << utils::formatPrint("{} -> {};\n", Name, Vertex->getName());
      }
    }
  }

  void clearGraphThreads() {
    rgs::for_each(Nodes,
                  [](auto &UniquePtr) { UniquePtr.get()->clearThreads(); });
  }

protected:
  std::vector<StoredNodePtr> Nodes;
};

template <std::input_iterator InputIt>
DirectedGraph(InputIt, InputIt)
    -> DirectedGraph<typename std::iterator_traits<InputIt>::value_type>;

} // namespace graphs
