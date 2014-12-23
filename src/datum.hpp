#ifndef ENTITY_DATUM_HPP_
#define ENTITY_DATUM_HPP_

#include "blob.hpp"
#include "path.hpp"
#include <vector>
#include <map>
#include <stdint.h>
#include <utility>

namespace entity {

class Datum {
public:
  // must call AddPath() to finish initialization
  Datum(const int entity_i, const int entity_o, const int count, 
      const int num_neg_sample) : entity_i_(entity_i), 
      entity_o_(entity_o), count_(count) {
    entity_i_grad_ = new Blob(entity::Context::dim_embedding());
    entity_o_grad_ = new Blob(entity::Context::dim_embedding());  

    neg_entity_id_.resize(num_neg_sample);
    neg_entity_grads_.resize(num_neg_sample);
    for (int i = 0; i < num_neg_sample; ++i) {
      neg_entity_grads_[i] = new Blob(entity::Context::dim_embedding());
    }
  };

  Datum(const int entity_i, const int entity_o, const int count, 
      Path* path, const int num_neg_sample) : entity_i_(entity_i), 
      entity_o_(entity_o), count_(count), category_path_(path), 
      category_path_nodes_(&path->category_nodes()) {
    entity_i_grad_ = new Blob(entity::Context::dim_embedding());
    entity_o_grad_ = new Blob(entity::Context::dim_embedding());  

    neg_entity_id_.resize(num_neg_sample);
    neg_entity_grads_.resize(num_neg_sample);
    for (int i = 0; i < num_neg_sample; ++i) {
      neg_entity_grads_[i] = new Blob(entity::Context::dim_embedding());
    }
    //positive_path_size = category_path_->category_nodes().size()
    //for (int i = 0; i < positive_path_size; ++i) {
    //  category_grads_.push_back(new Blob(
    //      entity::Context::dim_embedding(), entity::Context::dim_embedding()));
    //}
    ClearNegSamples();
  };

  ~Datum() {
     // TODO  
  };
  
  void AddPath(Path* path) {
    category_path_ = path; 
    category_path_nodes_ = &(path->category_nodes());
    ClearNegSamples();
  }

  const int entity_i() { return entity_i_; }
  const int entity_o() { return entity_o_; }
  const int count() { return count_; }

  Path* category_path() {
#ifdef DEBUG
    CHECK(category_path_ != NULL);
#endif
   return category_path_; 
  }
  
  void ClearNegSamples() {
    vector<Path*>().swap(neg_category_paths_);
    vector<Blob*>().swap(category_grads_);
    category_index_.clear();
    
    for (int i = 0; i < neg_entity_grads_.size(); ++i) {
      neg_entity_grads_[i]->ClearData();
    }
    for (int c_idx = 0; c_idx < category_path_nodes_->size(); ++c_idx) {
      category_index_[category_path_nodes_->at(c_idx)] = c_idx;
      category_grads_.push_back(new Blob(
          entity::Context::dim_embedding(), entity::Context::dim_embedding()));
    }
  }

  /// used in optimization 
  void AddNegSample(int neg_idx, int neg_entity_id, Path* path) {
#ifdef DEBUG
    CHECK_LT(neg_idx, neg_entity_id_.size());
#endif
    neg_entity_id_[neg_idx] = neg_entity_id;
    neg_category_paths_.push_back(path);

    const vector<int>& neg_category_path_nodes = path->category_nodes();
    for (int c_idx = 0; c_idx < neg_category_path_nodes.size(); ++c_idx) {
      const int category_id = neg_category_path_nodes[c_idx];
      if (category_index_.find(category_id) == category_index_.end()) {
        category_index_[category_id] = category_grads_.size();
        category_grads_.push_back(new Blob(
            entity::Context::dim_embedding(), entity::Context::dim_embedding()));
      }
    }
  }

  Blob* entity_i_grad() { return entity_i_grad_; }
  Blob* entity_o_grad() { return entity_o_grad_; }
   
  const int neg_entity(const int neg_idx) { return neg_entity_id_[neg_idx]; }

  vector<Path*>& neg_category_paths() { 
    return neg_category_paths_; 
  } 

  Path* neg_category_path(const int neg_idx) { 
    return neg_category_paths_[neg_idx]; 
  } 

  Blob* neg_entity_grad(const int neg_idx) { return neg_entity_grads_[neg_idx]; }

  Blob* category_grad(const int category_id) {
#ifdef DEBUG
    CHECK(category_index_.find(category_id) != category_index_.end());
#endif
    return category_grads_[category_index_[category_id]]; 
  }

  const vector<Blob*>& category_grads() { return category_grads_; }

  const map<int, int>& category_index() { return category_index_; }

private: 
  /// 
  int entity_i_;
  int entity_o_;
  int count_;
  Path* category_path_;
  vector<int>* category_path_nodes_;

  /// used in optimization 
  Blob* entity_i_grad_;
  Blob* entity_o_grad_;  
  
  // negative samples 
  vector<int> neg_entity_id_;
  // paths between entity_i_ and neg_entities
  vector<Path*> neg_category_paths_;
  vector<Blob*> neg_entity_grads_;

  // category_id => index in category_grad_
  // Note: includes categories in BOTH category_path_ and neg_category_paths_
  //   Clear it whenever negative sampling, and re-insert categories in 
  //   category_path_ 
  // TODO: or we can define seperate categroy_grads for category_path_
  map<int, int> category_index_;
  vector<Blob*> category_grads_;
};

}  // namespace entity

#endif
