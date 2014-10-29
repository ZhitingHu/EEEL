#pragma once

#include "common.hpp"
#include "blob.hpp"
#include <vector>


namespace entity {

class Path {
public:
  Path() {};
  ~Path() {};
  
  const vector<int>& category_nodes() { return category_nodes_; }
  const Blob* aggr_dist_metric() { return aggr_dist_metric_; }
  
private: 
  vector<int> category_nodes_;

  // aggregrated distance metrix
  Blob* aggr_dist_metric_; //TODO: update after solver.Update
};

}  // namespace entity
