#pragma once

#include <iterator>
#include <list>
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

  class list_range final {
    using list_iterator = std::list<DirGraphNode*>::iterator;

    list_iterator list_begin, list_end;
  public:
    list_range(list_iterator begin, list_iterator end): list_begin(begin), list_end(end) {}

    list_iterator begin() const { return list_begin; }
    list_iterator end() const { return list_end; }
    bool empty() const noexcept { return list_begin == list_end; }
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

  void addSuccessor(NodePtr Ptr) { Success.push_back(Ptr); }
  void removeSuccessor(NodePtr Ptr) { Success.remove(Ptr); }
  void addPredecessor(NodePtr Ptr) { Predecs.push_back(Ptr); }
  void removePredecessor(NodePtr Ptr) { Predecs.remove(Ptr); }
  
  DirGraphPtr getParent() const noexcept { return Parent; }

  list_range getSuccessors() const { return list_range {Success.begin(), Success.end()}; }
  list_range getPredecessors() const { return list_range {Predecs.begin(), Predecs.end()}; }
private:
  Data Dat;
  std::string Name;
  DirGraphPtr Parent;
  std::list<NodePtr> Success;
  std::list<NodePtr> Predecs;
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
      Vertices[Edge.second]->addPredecessor(Vertices[Edge.first]);
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
