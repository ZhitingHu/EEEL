#pragma once

#include "datum.hpp"
#include "context.hpp"
#include <vector>
#include <map>
#include <string>
#include <utility>

namespace entity {

class Dataset {
public:
  Dataset() {
    entity::Context& context = entity::Context::get_instance();
    num_neg_sample_ = context.get_int32("num_neg_sample");
  };

  ~Dataset() {};

  //// Add new Datum to dataset
  //void AddDatum(int entity_i, int entity_o, int count, Path* path) {
  //  Datum new_datum(entity_i, entity_o, count, path, num_neg_sample_);
  //  data_.push_back(new_datum);
  //  //AddPath(entity_i, entity_o, path);
  //  AddPair(entity_i, entity_o);
  //}
  //void AddDatum(int entity_i, int entity_o, int count) {
  //  Datum new_datum(entity_i, entity_o, count, num_neg_sample_);
  //  data_.push_back(new_datum);
  //  AddPair(entity_i, entity_o);
  //}
 
  // Must call AddPair() to finish adding new datum 
  void AddDatum(int entity_i, int entity_o, int count) {
    data_.push_back(make_pair(entity_i, entity_o));
    count_.push_back(count);
    AddPair(entity_i, entity_o);
  }

  //Datum* datum(int idx) { return &(data_[idx]); }
  Datum* datum(int idx) { 
    return new Datum(data_[idx].first, data_[idx].second, 
        count_[idx], num_neg_sample_); 
  }

  const set<int>& positive_entities(const int entity_i) { 
#ifdef DEBUG
    CHECK(entity_pairs_.find(entity_i) != entity_pairs_.end());
#endif
    return entity_pairs_[entity_i]; 
  }
 
  //const map<int, Path*>& positive_entity_path(const int entity_i) { 
  //#ifdef DEBUG
  //  CHECK(entity_pair_path_.find(entity_i) != entity_pair_path_.end());
  //#endif
  //  return entity_pair_path_[entity_i]; 
  //}
  //
  // Add path to dataset
  //void AddPath(const int entity_i, const int entity_o, Path* path){
  //  if (entity_pair_path_.find(entity_i) == entity_pair_path_.end()) {
  //    map<int, Path*> entity_o_path_map;
  //    entity_o_path_map[entity_o] = path;
  //    entity_pair_path_[entity_i] = entity_o_path_map;
  //  } else {
  //    entity_pair_path_[entity_i][entity_o] = path;
  //  }
  //}

  inline void AddPair(const int entity_i, const int entity_o) {
    entity_pairs_[entity_i].insert(entity_o);
  }

private: 

private:
  int num_neg_sample_;

  //vector<Datum> data_;
  // < (entity_i, entity_o) >
  vector<pair<int, int> > data_;
  // < count >
  vector<int> count_;

  // entity_i => (entitiy_o => path)
  //map<int, map<int, Path*> > entity_pair_path_;
  // entity_i => { entity_o }
  map<int, set<int> > entity_pairs_;
};

}  // namespace entity
