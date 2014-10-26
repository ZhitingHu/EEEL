#pragma once

#include "common.hpp"
#include <vector>


namespace entity {

class Path {
public:
  Path() {};
  ~Path() {};
  
  const vector<int>& category_nodes() { return category_nodes_; }

private: 
  vector<int> category_nodes_;
};

}  // namespace entity
