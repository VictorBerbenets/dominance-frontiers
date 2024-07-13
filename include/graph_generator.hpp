#pragma once

#include <vector>
#include <algorithm>
#include <fstream>
#include <random>
#include <string_view>

namespace graphs {

class DirGraphBuilder final {
  using GeneratorType = std::mt19937;
 public:
  using size_type = std::size_t;
  
  static constexpr size_type DefNodesCount  = 5;
  static constexpr size_type DefEdgeCount = 1;
  static constexpr size_type MaxNodesCount  = 15;
  static constexpr size_type MaxEdgeCount = 5;
  
  // Generate Graph into given file:
  // Example graph print (v is a node's name):
  //    v_1 --> v_2
  //    v_2 --> v_3
  //    v_3 --> v_1
  //    v_1 --> v_1

  static void generateGraph(std::ofstream &OutFile,
                      size_type NodesCount = DefNodesCount,
                      size_type EdgeCount = DefEdgeCount,
                      std::string_view NodeName = "BB") {
    if (EdgeCount > NodesCount)
      EdgeCount = 1;

    if (NodesCount == 0 || NodesCount > MaxNodesCount)
      NodesCount = DefNodesCount;

    if (EdgeCount > MaxEdgeCount)
      EdgeCount = DefEdgeCount + 1;
    
    // create random generator
    std::random_device Device;
    GeneratorType Engine {Device()};
    
    for (size_type NodeNum = 1; NodeNum <= NodesCount; ++NodeNum) {
      std::vector<size_type> PairNodes(NodesCount);
      std::iota(PairNodes.begin(), PairNodes.end(), 1);
      for (size_type EdgeNum = 0, EdgesCount = getRandomUnsInt(Engine, 1, EdgeCount);
            EdgeNum < EdgesCount; ++EdgeNum) {
        auto CellNode = getRandomUnsInt(Engine, 0, PairNodes.size() - 1);
        printEdge(OutFile, std::string(NodeName) + "_", NodeNum, PairNodes[CellNode]);
        PairNodes.erase(PairNodes.begin() + CellNode);
      }
    }
  }

 private:
  static void printEdge(std::ofstream &OutFile, const std::string &NodeName,
             size_type NodeFrom, size_type NodeTo) {
    OutFile << NodeName + std::to_string(NodeFrom) << " --> " << 
          NodeName + std::to_string(NodeTo) << std::endl; 
  }

  static size_type getRandomUnsInt(GeneratorType &Gener,
                            size_type LowInt, size_type MaxInt) {
    std::uniform_int_distribution<size_type> IntDistr {LowInt, MaxInt};
    return IntDistr(Gener);
  }
};


} // graphs
