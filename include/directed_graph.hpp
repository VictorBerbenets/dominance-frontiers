#pragma once

#include <std::vector>
#include <memory>

namespace graphs {

class DirectedGraph {
 public:
  virtual ~DirectedGraph() {}
 private:
  std::vector<std::unique_ptr> nodes_;
};

} // namespace graphs

