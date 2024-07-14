#pragma once

#include "directed_graph.hpp"

namespace graphs {

template <typename T> class DomTreeGraph : public DirectedGraph {
public:
  template <std::forward_iterator ForwIt>
    requires requires(ForwIt It) {
      { *It } -> std::convertible_to<EdgeType>;
    }
  DomTreeGraph(ForwIt FBegin, ForwIt FEnd) : DirectedGraph(FBegin, FEnd) {}

private:
  NodeTypePtr Root;
};

} // namespace graphs
