#pragma once

#include <memory>
#include <vector>
#include <list>

namespace graphs {

class DirectedGraph;

template <typename Data = int>
class DirGraphNode {
  using DirGraphPtr = DirectedGraph*;

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
  
  DirGraphNode(const std::string &Name = "", DirGraphPtr Parent = nullptr): Name(Name),
   Parent(Parent) {}
  
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

class DirectedGraph {
  using NodePtr = std::unique_ptr<DirGraphNode<int>>;
public:
  virtual ~DirectedGraph() {}

private:
  std::vector<NodePtr> Nodes;
};

} // namespace graphs
