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
  void Add_Datum(int entity_1_id, int entity_2_id){
    Datum new_Datum(entity_1_id, entity_2_id);  //make sure constuctor
    data.push_back(new_Datum);
  }

  // Read Datum from dataset
  Datum* Get_Datum_adr(int idx) const { return (Datum*) &data[idx]; }

  // move to ee_engine for simplicity
  // for negative sampling  
  //const bool Cooccur(int entity_1_id, int entity_2_id){
  //  int32_t neg_entity_id = static_cast <int> (rand()) % static_cast <int> (num_entity_);
  //  return neg_entity_id == entity_1_id;
  //}

  const Path* GetPath(int entity_1_id, int entity_2_id){
    //TODO:
    // temp for test only
    return (Path* ) &entity_1_id;
  };

private: 
  vector<Datum> data;

  // entity_1_id => (entity_2_id => path)
  map<int, map<int, Path> > entity_pair_path_;
};

}  // namespace entity
