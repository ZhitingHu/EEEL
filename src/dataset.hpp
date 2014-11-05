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
  void Add_Datum(int entity_1_id, int entity_2_id, int count){
    Datum new_Datum(entity_1_id, entity_2_id, count);  //make sure constuctor
    data.push_back(new_Datum);
  }

  // Read Datum from dataset
  Datum* Get_Datum_adr(int idx) const { return (Datum*) &data[idx]; }

  // Add path to dataset
  void Add_Path(int idx, int entity_2_id, Path* entity_pair_path){
    map<int, Path> temp_map; 
    temp_map[entity_2_id] = *entity_pair_path;
    entity_pair_path_[idx] = temp_map;
    //entity_pair_path_.insert(temp_map);
    /*
    map<int, map<int, Path> > entity_pair_path_;
    if (entity_pair_path_.find(i) == entity_pair_path_.end()) {
      entity_pair_path_[i] = map;
    } else {
      entity_pair_path_[i][o] = path;
    }
    */
  }

  // move to ee_engine for simplicity
  // for negative sampling  
  //const bool Cooccur(int entity_1_id, int entity_2_id){
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
  vector<Datum> data;

  // entity_1_d => (entitiy_2_id => path)
  map<int, map<int, Path> > entity_pair_path_;
  map<int, int> ee;
};

}  // namespace entity
