#pragma once

#include <algorithm>
#include <fstream>
#include <random>
#include <set>
#include <string_view>
#include <vector>

#include "utils.hpp"

namespace graphs {

class DirGraphBuilder final {
  using GeneratorType = std::mt19937;

public:
  using size_type = std::size_t;

  static constexpr std::string_view DefNodeName = "BB";
  static constexpr size_type DefNodeNum = 5;
  static constexpr size_type DefEdgeNum = 1;

  // Generate txt graph into given file:
  // Example graph print (BB is a node's name):
  //    BB_1 --> BB_2
  //    BB_2 --> BB_3
  //    BB_3 --> BB_1
  //    BB_1 --> BB_1

  // Note: if EdgeNum >= NodeNum - got UB possibility
  static void generateGraph(std::ofstream &OutFile,
                            size_type NodeNum = DefNodeNum,
                            size_type EdgeNum = DefEdgeNum,
                            std::string_view NodeName = DefNodeName) {
    if (EdgeNum == 0)
      return;
    // create random generator
    std::random_device Device;
    GeneratorType Engine{Device()};

    std::vector<std::vector<size_type>> Graph(NodeNum);
    std::set<size_type> FreeNodes;
    std::generate_n(std::inserter(FreeNodes, FreeNodes.end()), NodeNum - 1,
                    [&FreeNodes] { return FreeNodes.size() + 1; });
    // Making tree graph
    printEdge(OutFile, NodeName, 0, 1);
    Graph[0].push_back(1);
    FreeNodes.erase(FreeNodes.begin());
    for (size_type NodeCount = 1; NodeCount < Graph.size(); ++NodeCount) {
      if (FreeNodes.empty())
        break;
      auto SuccessorsNum = getRandomUnsInt(Engine, 1, EdgeNum);
      for (size_type SuccessorsCount = 0;
           SuccessorsCount < SuccessorsNum && !FreeNodes.empty();
           ++SuccessorsCount) {
        auto Successor = *FreeNodes.begin();
        printEdge(OutFile, NodeName, NodeCount, Successor);
        // saving Successor
        Graph[NodeCount].push_back(Successor);
        FreeNodes.erase(FreeNodes.begin());
      }
    }
    // Adding random edges (having high possibility to make loops)
    std::vector<size_type> AllNodes(NodeNum - 1);
    std::iota(AllNodes.begin(), AllNodes.end(), 1);
    for (size_type NodeCount = 1; NodeCount < Graph.size(); ++NodeCount) {
      if (size_type EdgeCount = Graph[NodeCount].size(); EdgeCount < EdgeNum) {
        auto AddEdgesCount = getRandomUnsInt(Engine, 0, EdgeNum - EdgeCount);
        std::vector<size_type> Diff;
        std::set_difference(AllNodes.begin(), AllNodes.end(),
                            Graph[NodeCount].begin(), Graph[NodeCount].end(),
                            std::back_inserter(Diff));
        std::erase(Diff, NodeCount);
        for (EdgeCount = 0; EdgeCount < AddEdgesCount && !Diff.empty();
             ++EdgeCount) {
          auto CellId = getRandomUnsInt(Engine, 0, Diff.size() - 1);
          auto NodeTo = Diff[CellId];
          if (NodeCount != NodeTo)
            printEdge(OutFile, NodeName, NodeCount, NodeTo);
          Diff.erase(Diff.begin() + CellId);
        }
      }
    }
  }

private:
  static void printEdge(std::ofstream &OutFile, std::string_view NodeName,
                        size_type NodeFrom, size_type NodeTo) {

    auto Str = utils::formatPrint("{}_{} --> {}_{}\n", NodeName, NodeFrom,
                                  NodeName, NodeTo);
    OutFile << Str;
  }

  static size_type getRandomUnsInt(GeneratorType &Gener, size_type LowInt,
                                   size_type MaxInt) {
    std::uniform_int_distribution<size_type> IntDistr{LowInt, MaxInt};
    return IntDistr(Gener);
  }
};

} // namespace graphs
