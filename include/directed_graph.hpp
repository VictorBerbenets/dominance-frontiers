#pragma once

#include <concepts>
#include <cstdlib>
#include <filesystem>
#include <iterator>
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>
#include <ranges>

namespace graphs {

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
    if (auto RmIter = std::ranges::find(Successors, Ptr); RmIter != Successors.end()) {
      (*RmIter)->removePredecessor(this);
      Successors.erase(RmIter);
    }
  }
  void addPredecessor(NodePtr Ptr) { Predecessors.push_back(Ptr); }
  void removePredecessor(NodePtr Ptr) { std::erase(Predecessors, Ptr); }

  DirGraphPtr getParent() const noexcept { return Parent; }

  auto getSuccessors() const {
    return std::ranges::subrange {Successors};
  }
  auto getPredecessors() const {
    return std::ranges::subrange {Predecessors};
  }

  auto getSuccessors() {
    return std::ranges::subrange {Successors};
  }

  auto getPredecessors() {
    return std::ranges::subrange {Predecessors};
  }

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

  void clearThreads() {
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

namespace fs = std::filesystem;

namespace detail {

template <typename T>
auto dumpIntoPngImpl(const DirectedGraph<T> &DirGraph,
                            fs::path PathToCreate, std::string FileName) {
  static const char *DotSubstrEnd = ".dot";

  fs::path FullPath = PathToCreate / (FileName + DotSubstrEnd);
  std::ofstream DotFile(FullPath);
  DirGraph.dumpInDotFormat(DotFile);

  std::string DotCommand = "dot -Tpng ";
  DotCommand.append(FullPath);
  DotCommand.append(" -o ");
  DotCommand.append(PathToCreate.append(FileName).replace_extension(".png"));
  return DotCommand;
}

} // namespace detail

template <typename T>
void dumpIntoPng(const DirectedGraph<T> &DirGraph, fs::path PathToCreate,
                 std::string FileName) {
  auto DotCommand = detail::dumpIntoPngImpl(DirGraph, PathToCreate, FileName);
  std::system(DotCommand.c_str());
}

template <typename T>
  requires std::is_default_constructible_v<T>
class DirectedGraph {
public:
  using value_type = T;

protected:
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

    auto HandleEdge = [&](const EdgeType &Edge) {
      for (auto V : {Edge.first, Edge.second}) {
        if (!Vertices.contains(V)) {
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

  void dumpInDotFormat(std::ofstream &DotDump) const {
    static constexpr std::string_view DotHeader =
        "digraph List {\n"
        "\tdpi = 100;\n"
        "\tfontname = \"Comic Sans MS\";\n"
        "\tfontsize = 20;\n"
        "\trankdir  = TB;\n"
        "graph [fillcolor = lightgreen, ranksep = 1.3, nodesep = 0.5,"
        "style = \"rounded\", color = green, penwidth = 2];\n"
        "edge [color = black, arrowhead = diamond, arrowsize = 1, penwidth = "
        "1.2];\n";

    DotDump << DotHeader;
    for (const auto &Ptr : Nodes) {
      auto Name = Ptr.get()->getName();
      DotDump << Name
              << "[shape = Mrecord, style = filled,"
                 "fillcolor = \"#B91FAF\"];"
              << std::endl;
      for (auto Vertex : Ptr.get()->getSuccessors()) {
        DotDump << Name << " -> " << Vertex->getName() << ";" << std::endl;
      }
    }
    DotDump << "}\n";
  }

  // access random graph node ptr
  NodeTypePtr getNodePtr() const noexcept { return Nodes.front().get(); }

protected:
  std::vector<StoredNodePtr> Nodes;
};

template <std::input_iterator InputIt>
DirectedGraph(InputIt, InputIt)
    -> DirectedGraph<typename std::iterator_traits<InputIt>::value_type>;

} // namespace graphs
