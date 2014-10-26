#pragma once

#include "path.hpp"
#include <vector>
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
  int entity_i_;
  int entity_o_;
  int count_;
  Path* category_path_;

};

}  // namespace entity
