#include <iostream>
#include <stdexcept>
#include <fstream>

#include "graph_generator.hpp"

int main(int args, char **argv) {
  if (args < 2) 
      throw std::runtime_error {"invalid input for graph generation program"};
  std::ofstream GraphFile {argv[1]};
  switch (args) {
    case 2:
      graphs::DirGraphBuilder::generateGraph(GraphFile);
      break;
    case 3:
      graphs::DirGraphBuilder::generateGraph(GraphFile, std::stoul(argv[2]));
      break;
    case 4:
      graphs::DirGraphBuilder::generateGraph(GraphFile, std::stoul(argv[2]),
                                             std::stoul(argv[3]));
      break;
    case 5:
      graphs::DirGraphBuilder::generateGraph(GraphFile, std::stoul(argv[2]),
                                             std::stoul(argv[3]), argv[4]);
      break;
    default:
      throw std::runtime_error {"invalid input for graph generation program"};
  }
}

