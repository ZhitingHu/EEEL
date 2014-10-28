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
  map<int, int> update_entity_id_index_;
  vector<Blob*> update_entity_values_;
  // category_id => index in update_category_values_ 
  map<int, int> update_category_id_index_;
  vector<Blob*> update_category_values_;
};

}  // namespace entity
