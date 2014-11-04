#ifndef ENTITY_PATH_HPP_
#define ENTITY_PATH_HPP_

#include "common.hpp"
#include "blob.hpp"

#include <vector>
#include <map>

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

  void AddCategoryNode(const int category_id, const float weight) {
#ifdef DEBUG
    CHECK(category_node_weights_.find(category_id) 
        == category_node_weights_.end());
#endif
    category_nodes_.push_back(category_id);
    category_node_weights_[category_id] = weight;
  }

  void ScaleCategoryWeights(const float scale) {
    map<int, float>::iterator it = category_node_weights_.begin();
    for (; it != category_node_weights_.end(); ++it) {
      it->second *= scale;
    }
  }

  void IncCategoryNodeWeight(const int category_id, const int weight = 1) {
#ifdef DEBUG
    CHECK(category_node_weights_.find(category_id) 
        != category_node_weights_.end());
#endif
    category_node_weights_[category_id] += weight;
  }

  const vector<int>& category_nodes() const { return category_nodes_; }
  const Blob* aggr_dist_metric() const { return aggr_dist_metric_; }
  
 
private: 
  vector<int> category_nodes_;
  // category_id => weight in the path
  map<int, float> category_node_weights_;

  // aggregrated distance metrix
  Blob* aggr_dist_metric_;
};

}  // namespace entity

#endif
