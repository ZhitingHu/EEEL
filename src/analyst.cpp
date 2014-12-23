// Date: 2014.10.26
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "util.hpp"
#include "string_util.hpp"
#include "analyst.hpp"

#include <string>
#include <fstream>
#include <iterator>
#include <vector>
#include <stdint.h>
#include <algorithm>

namespace entity {

  using namespace util;

struct sort_comparator {
  bool operator()(const std::pair<int,float> &lhs, const std::pair<int,float> &rhs) {
      return lhs.second < rhs.second;
  }
};

Analyst::Analyst(const string& snapshot_path, const int iter) {
  ee_engine_ = new EEEngine();
  ee_engine_->ReadData();

  solver_ = new Solver();
  solver_->Restore(snapshot_path, iter);
}

void Analyst::ComputeNearestNeibors(const int top_k, 
    const string& output_path) {
  LOG(ERROR) << "Computing nearest neibors";
  ofstream output;
  output.open((output_path + "/nearest_neibors").c_str());
  for (int e_id = 0; e_id < solver_->num_entity(); ++e_id) {
    vector<pair<int, float> > nearest_entities;
    ComputeEntityNearestNeibors(e_id, nearest_entities);
    for (int rank = 0; rank < top_k; ++rank) {
      output << nearest_entities[rank].first << ":" 
          << nearest_entities[rank].second << " ";
    }
    output << endl;
  }
  output.close();
}

void Analyst::ComputeEntityNearestNeibors(const int entity, 
    vector<pair<int, float> >& nearest_entities) {
  for (int e_id = 0; e_id < solver_->num_entity(); ++e_id) {
    if (e_id == entity) { continue; }
    Path* path = ee_engine_->entity_category_hierarchy()->
        FindPathBetweenEntities(entity, e_id);
    path->RefreshAggrDistMetric(solver_->categories());
    float dist = solver_->ComputeDist(entity, e_id, path);
    nearest_entities.push_back(pair<int, float>(e_id, dist));
  }
  // sort
  std::sort(nearest_entities.begin(), nearest_entities.end(), 
      sort_comparator()); 
}


} // namespace entity
