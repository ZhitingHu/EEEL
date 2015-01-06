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
    category_node_weights_.clear();
    delete aggr_dist_metric_; 
  };
  
  void RefreshAggrDistMetric(const vector<Blob*>& categories) {
    if (!aggr_dist_metric_) {
      aggr_dist_metric_ = new Blob(
          entity::Context::dim_embedding(), entity::Context::dim_embedding());
    } else {  
      aggr_dist_metric_->ClearData();
    }
    for (int c_idx = 0; c_idx < category_nodes_.size(); ++c_idx) {
      const int category_id = category_nodes_[c_idx];
#ifdef DEBUG
      CHECK(category_node_weights_.find(category_id) 
          != category_node_weights_.end());
#endif
      // weighted
      aggr_dist_metric_->Accumulate(categories[category_id], 
          category_node_weights_[category_id]);
    }
  }

  void AddCategoryNode(const int category_id, const float weight) {
#ifdef DEBUG
    CHECK(category_node_add_times_[category_id] < 2);
    category_node_add_times_[category_id]++;
#endif
    category_nodes_.push_back(category_id);
    category_node_weights_[category_id] += weight;
  }

  //TODO
  void ScaleCategoryWeights(const float scale, const float scale_2) {
    map<int, float>::iterator it = category_node_weights_.begin();
    for (; it != category_node_weights_.end(); ++it) {
      it->second *= scale * scale_2; //weighted
    }
    scale_2_ = scale_2;
  }
  float scale_2() { return scale_2_; }

  vector<int>& category_nodes() { return category_nodes_; }
  const float category_node_weight(const int category_id) {
#ifdef DEBUG
    CHECK(category_node_weights_.find(category_id) 
        != category_node_weights_.end());
#endif
    return category_node_weights_[category_id];
  }
  const Blob* aggr_dist_metric() const { return aggr_dist_metric_; }
  
 
private: 
  vector<int> category_nodes_;
  // category_id => weight in the path
  map<int, float> category_node_weights_;

#ifdef DEBUG
  map<int, int> category_node_add_times_;
#endif

  // aggregrated distance metrix
  Blob* aggr_dist_metric_;
  
  //TODO
  float scale_2_;
};

}  // namespace entity

#endif
