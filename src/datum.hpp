#pragma once

#include <vector>
#include <cstdint>
#include <utility>

namespace entity {

class Datum {
public:
  Datum();
  ~Datum();
  
  const int entity_i() { return entity_i_; }
  const int entity_j() { return entity_j_; }
  const int count() { return count_; }
  const vector<int>& category_path() { return category_path_; }

private: 
  int entity_i_;
  int entity_j_;
  int count_;
  vector<int> category_path_;
};

}  // namespace entity
