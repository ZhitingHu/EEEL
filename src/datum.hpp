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
  
  Blob* entity_i_grad() { return entity_i_grad_; }
  
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
  vector<Path*> entity_i_neg_paths_;
  vector<Blob*> entity_grad_;
  vector<int> category_id_;
  vector<Blob*> category_grad_;
};

}  // namespace entity
