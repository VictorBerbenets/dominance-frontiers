#pragma once

#include <iterator>
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace graphs {

template <typename T>
  requires std::is_default_constructible_v<T>
class DirectedGraph;

template <typename Data = int>
class DirGraphNode {
  using DirGraphPtr = DirectedGraph<Data> *;

  class range final {
    using range_iterator = std::vector<DirGraphNode *>::iterator;

    range_iterator range_begin, range_end;

  public:
    range(range_iterator begin, range_iterator end)
        : range_begin(begin), range_end(end) {}

    range_iterator begin() const { return range_begin; }
    range_iterator end() const { return range_end; }
    bool empty() const noexcept { return range_begin == range_end; }
  };

public:
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
    if (auto RmIter = std::find(Successors, Ptr); RmIter != Successors.end()) {
      (*RmIter)->removePredecessor(this);
      Successors.erase(RmIter);
    }
  }
  void addPredecessor(NodePtr Ptr) { Predecessors.push_back(Ptr); }
  void removePredecessor(NodePtr Ptr) { std::erase(Predecessors, Ptr); }

  DirGraphPtr getParent() const noexcept { return Parent; }

  range getSuccessors() const {
    return range{Successors.begin(), Successors.end()};
  }
  range getPredecessors() const {
    return range{Predecessors.begin(), Predecessors.end()};
  }

private:
  Data Dat;
  std::string Name;
  DirGraphPtr Parent;
  std::vector<NodePtr> Successors;
  std::vector<NodePtr> Predecessors;
};

template <typename T>
  requires std::is_default_constructible_v<T>
class DirectedGraph {
  using value_type = T;
  using NodeType = DirGraphNode<value_type>;
  using NodeTypePtr = NodeType *;
  using StoredNodePtr = std::unique_ptr<NodeType>;
  using EdgeType = std::pair<std::string, std::string>;

public:
  template <std::input_iterator InputIt>
    requires requires(InputIt It) {
      { *It } -> std::convertible_to<EdgeType>;
    }
  DirectedGraph(InputIt BeginIt, InputIt EndIt) {
    std::unordered_map<std::string, NodeType *> Vertices;

    auto HandleEdge = [&Vertices, this](const EdgeType &Edge) {
      for (auto V : {Edge.first, Edge.second}) {
        if (Vertices.find(V) == Vertices.end()) {
          Nodes.push_back(std::make_unique<NodeType>(value_type(), V, this));
          auto RawPtr = Nodes.back().get();
          Vertices.emplace(V, RawPtr);
        }
      }
      Vertices[Edge.first]->addSuccessor(Vertices[Edge.second]);
    };

    for (; BeginIt != EndIt; ++BeginIt) {
      HandleEdge(*BeginIt);
    }
  }

  virtual ~DirectedGraph() {}

  void dumpInDotFormat() const {}

  // access random graph node ptr
  NodeTypePtr getNodePtr() const noexcept { return Nodes.front().get(); }

private:
  std::vector<StoredNodePtr> Nodes;
};

template <std::input_iterator InputIt>
DirectedGraph(InputIt, InputIt)
    -> DirectedGraph<typename std::iterator_traits<InputIt>::value_type>;

} // namespace graphs
