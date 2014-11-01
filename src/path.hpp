#ifndef ENTITY_PATH_HPP_
#define ENTITY_PATH_HPP_

#include "common.hpp"
#include "blob.hpp"

#include <vector>

namespace entity {

class Path {
public:
  Path() : aggr_dist_metric_(NULL) {};

  ~Path() {
    vector<int>().swap(category_nodes_);
    delete aggr_dist_metric_; 
  };
  
  void RefreshAggrDistMetric(const vector<Blob*> categories) {
    if (!aggr_dist_metric_) {
      aggr_dist_metric_ = new Blob(
          entity::Context::dim_embedding(), entity::Context::dim_embedding());
    }  
    aggr_dist_metric_->ClearData();
    for (int c_idx = 0; c_idx < category_nodes_.size(); ++c_idx) {
      aggr_dist_metric_->Accumulate(categories[category_nodes_[c_idx]]);
    }
  }

  const vector<int>& category_nodes() const { return category_nodes_; }
  const Blob* aggr_dist_metric() const { return aggr_dist_metric_; }
 
private: 
  vector<int> category_nodes_;

  // aggregrated distance metrix
  Blob* aggr_dist_metric_;
};

}  // namespace entity

#endif
