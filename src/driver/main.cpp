#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>

#include "directed_graph.hpp"

static std::vector<std::pair<std::string, std::string>>
getGraphEdges(std::ifstream &Ifs) {
  static constexpr std::string_view Arrow = " --> ";

  std::vector<std::pair<std::string, std::string>> Edges;
  std::string Line;
  while (std::getline(Ifs >> std::ws, Line)) {

    auto FirstPos = Line.find_first_of(Arrow);
    Edges.emplace_back(Line.substr(0, FirstPos),
                       Line.substr(FirstPos + Arrow.size(), std::string::npos));
  }

  return Edges;
}

int main(int args, char **argv) {
  if (args < 2) {
    throw std::runtime_error{"input error: expected file name"};
  }

  std::ifstream GraphFile{argv[1]};
  auto Edges = getGraphEdges(GraphFile);
  graphs::DirectedGraph gr(Edges.cbegin(), Edges.cend());
}
