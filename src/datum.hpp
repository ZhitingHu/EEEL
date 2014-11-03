#ifndef ENTITY_DATUM_HPP_
#define ENTITY_DATUM_HPP_

#include "blob.hpp"
#include "path.hpp"
#include <vector>
#include <map>
#include <cstdint>
#include <utility>

namespace entity {

class Datum {
public:
  Datum(int entity_i, int entity_o) : entity_i_(entity_i), entity_o_(entity_o){};
  ~Datum() {};
  
  const int entity_i() { return entity_i_; }
  const int entity_o() { return entity_o_; }
  const int count() { return count_; }
  Path* category_path() { return category_path_; }
  
  /// used in optimization 
  void AddNegSample(const int neg_entity_id, const Path* path) {
    //TODO
    neg_entity_id_.push_back(neg_entity_id);
    //neg_category_paths_.push_back(path);
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

  /// used in optimization 
  Blob* entity_i_grad_;
  Blob* entity_o_grad_;  

  // negative samples 
  vector<int> neg_entity_id_;
  // paths between entity_i_ and neg_entities
  vector<Path*> neg_category_paths_;
  vector<Blob*> neg_entity_grads_;

  // category_id => index in category_grad_
  map<int, int> category_index_;
  vector<Blob*> category_grads_;
};

}  // namespace entity

#endif
