#pragma once

#include "blob.hpp"
#include "path.hpp"
#include <vector>
#include <map>
#include <cstdint>
#include <utility>

namespace entity {

class Datum {
public:
  Datum() {};
  ~Datum() {};
  
  const int entity_i() { return entity_i_; }
  const int entity_o() { return entity_o_; }
  const int count() { return count_; }
  const Path* category_path() { return category_path_; }

private: 
  /// 
  int entity_i_;
  int entity_o_;
  int count_;
  Path* category_path_;

  /// used in optimization 

  // entity_id => index in update_entity_values_
  //map<int, int> entity_id_index_;
  // category_id => index in update_category_values_ 
  //map<int, int> category_id_index_;

  // negative samples 
  vector<int> neg_entity_id_;
  vector<Path*> entity_i_neg_entity_paths_;
  // gradients 
  vector<Blob*> entity_gradient_;
  
  //
  vector<int> category_id_;
  vector<Blob*> category_gradient_;
};

}  // namespace entity
