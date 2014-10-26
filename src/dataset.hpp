#pragma once

#include "datum.hpp"
#include <vector>
#include <map>
#include <string>
#include <utility>

namespace entity {

class Dataset {
public:
  Dataset() {};
  ~Dataset() {};
  
  // 
  void ReadData(const string& file_name);

  // for negative sampling
  const bool Cooccur(int entity_1_id, int entity_2_id);

  const Path* GetPath(int entity_1_id, int entity_2_id);

private: 
  vector<Datum> data;

  // entity_1_id => (entity_2_id => path)
  map<int, map<int, Path> > entity_pair_path_;
};

}  // namespace entity
