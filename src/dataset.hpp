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
  
  // move to ee_engine for simplicity
  //void ReadData(const string& file_name);

  // Add new Datum to dataset
  void AddDatum(int entity_i, int entity_o, int count){
    Datum new_datum(entity_i, entity_o, count);  //make sure constuctor
    data.push_back(new_datum);
  }

  Datum* datum(int idx) const { return &(data_[idx]); }
 
  const map<int, map<int, Path*> >& entity_pair_path() { 
    return entity_pair_path_; 
  }
 
  // Add path to dataset
  void AddPath(const int entity_i, const int entity_o, Path* path){
    //map<int, Path> temp_map; 
    //temp_map[entity_2_id] = *entity_pair_path;
    //entity_pair_path_[idx] = temp_map;
    if (entity_pair_path_.find(entity_i) == entity_pair_path_.end()) {
      map<int, Path*> entity_o_path_map;
      entity_o_path_map[entity_o] = path;
      entity_pair_path_[entity_i] = entity_o_path_map;
    } else {
      entity_pair_path_[entity_i][entity_o] = path;
    }
  }

  // move to ee_engine for simplicity
  // for negative sampling  
  //inline bool Cooccur(int entity_1_id, int entity_2_id){
  //  int32_t neg_entity_id = static_cast <int> (rand()) % static_cast <int> (num_entity_);
  //  return neg_entity_id == entity_1_id;
  //}

  // merged to hierarchy.FindPathBetweenEntities()
  //const Path* GetPath(int entity_1_id, int entity_2_id){
  //  //TODO:
  //  // temp for test only
  //  return (Path* ) &entity_1_id;
  //};

private: 
  vector<Datum> data_;

  // entity_1_d => (entitiy_2_id => path)
  map<int, map<int, Path*> > entity_pair_path_;

  // TODO what's this for? @hzt
  map<int, int> ee;
};

}  // namespace entity
