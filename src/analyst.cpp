// Date: 2014.10.26
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "util.hpp"
#include "string_util.hpp"
#include "analyst.hpp"
#include "dataset.hpp"

#include <string>
#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>
#include <stdint.h>
#include <algorithm>
#include <omp.h>

namespace entity {

  using namespace util;

struct DesSortIntFloatPairs{
  bool operator()(
      const pair<int,float> &lhs, 
      const pair<int,float> &rhs) {
    return lhs.second < rhs.second;
  }
};

struct DesSortIntPairPairs {
  bool operator()(
      const pair<int, pair<float, float> > &lhs, 
      const pair<int, pair<float, float> > &rhs) {
    return lhs.second.first < rhs.second.first;
  }
};


Analyst::Analyst(const string& resume_path, const int iter) {
  ee_engine_ = new EEEngine();
  ee_engine_->ReadData();

  solver_ = new Solver();
  solver_->Restore(resume_path, iter);

  LOG(INFO) << "Read id-name maps";
  entity::Context& context = entity::Context::get_instance();
  const string& dataset_path = context.get_string("dataset_path");
  const string& entity_filename = context.get_string("entity_filename");
  ReadIdNames(dataset_path + "/" + entity_filename, entity_id_names_);
  const string& category_filename = context.get_string("category_filename");
  ReadIdNames(dataset_path + "/" + category_filename, category_id_names_);
}

void Analyst::ReadIdNames(const string& filename, map<int, string>& id_names) {
  ifstream input(filename.c_str());
  CHECK(input.is_open()) << "Fail to open " << filename;
  int id = 0;
  string name;
  while (getline(input, name)) {
    id_names[id] = name;
    id++;
  }
  input.close();
}

//==============================
// NN by specified categories
//==============================
void Analyst::ComputeNearestNeiborsByCategories(
    const int top_k, const vector<int>& candidate_entities,
    const vector<vector<vector<int> > >& categories, const string& output_path) {
  LOG(ERROR) << "Computing nearest neibors by categories";
  ofstream output;
  output.open((output_path + "/nearest_neibors_by_categories").c_str());
  CHECK(output.is_open());

  Dataset* train_data = ee_engine_->train_data();
  int counter = 0;
  LOG(INFO) << "Number of candidate entities: " << candidate_entities.size();
  for (int idx = 0; idx < candidate_entities.size(); ++idx) {
    const int e_id = candidate_entities[idx];
    CHECK(entity_id_names_.find(e_id) != entity_id_names_.end());
    output << entity_id_names_[e_id] << "\t" << e_id << "\n";
    LOG(INFO) << "entity " << e_id << "\t" << entity_id_names_[e_id];

    const vector<vector<int> >& entity_categories = categories[idx];

    for (int cvec_i = 0; cvec_i < entity_categories.size(); ++cvec_i) {
      const vector<int>& target_categories = entity_categories[cvec_i]; 
      for (int c_i = 0; c_i < target_categories.size(); ++c_i) {
        const int category_id = target_categories[c_i];
        output << category_id_names_[category_id] << ":" << category_id << "\t";
      }
      output << "\n";

      // Compute NN
      vector<pair<int, float> > nearest_entities;
      ComputeEntityNearestNeiborsByCategories(
          e_id, top_k, target_categories, nearest_entities);
      // Output
      for (int rank = 0; rank < top_k; ++rank) {
        // TODO
        int occur_cnt = 0;
        if (train_data->positive_entities(e_id).find(nearest_entities[rank].first)
            != train_data->positive_entities(e_id).end()) {
          occur_cnt++;
        }
        if (train_data->positive_entities(nearest_entities[rank].first).find(e_id)
            != train_data->positive_entities(nearest_entities[rank].first).end()) {
          occur_cnt++;
        }
        
        output << entity_id_names_[nearest_entities[rank].first] << ":" 
            << nearest_entities[rank].first << ":" 
            << nearest_entities[rank].second << ":" << occur_cnt << "\t";
      }
      output << endl;

      counter++;
      if (counter % (candidate_entities.size() / 10 + 1) == 0) {
        std::cout << "." << std::flush;
      }
    }
    std::cout << std::endl;
  }

  output.flush();
  output.close();
}

void Analyst::ComputeEntityNearestNeiborsByCategories(
    const int entity, const int top_k, const vector<int>& categories,
    vector<pair<int, float> >& nearest_entities) {
  Path* path = new Path();
  for (int i = 0; i < categories.size(); ++i) {
    path->AddCategoryNode(categories[i], 1.0);
  } 
  path->RefreshAggrDistMetric(solver_->categories());
#ifdef OPENMP
  #pragma omp parallel for
#endif
  for (int e_id = 0; e_id < solver_->num_entity(); ++e_id) {
    if (e_id == entity) { continue; }
    float dist = solver_->ComputeDist(entity, e_id, path);

#ifdef OPENMP
    #pragma omp critical
#endif
    {  
    nearest_entities.push_back(pair<int, float>(e_id, dist));
    }

  }
  delete path;

  // sort
  std::partial_sort(nearest_entities.begin(), nearest_entities.begin() + top_k, 
      nearest_entities.end(), DesSortIntFloatPairs());
}


//==============================
// NN in the whole hierarchy
//==============================
void Analyst::ComputeNearestNeibors(const int top_k, 
    const vector<int>& candidate_entities, const string& output_path) {
  LOG(ERROR) << "Computing nearest neibors";
  ofstream output;
  output.open((output_path + "/nearest_neibors").c_str());
  CHECK(output.is_open());

  Dataset* train_data = ee_engine_->train_data();
  int counter = 0;
  LOG(INFO) << "Number of candidate entities: " << candidate_entities.size();
  for (int idx = 0; idx < candidate_entities.size(); ++idx) {
    const int e_id = candidate_entities[idx];
    vector<pair<int, pair<float, float> > > nearest_entities;
    ComputeEntityNearestNeibors(e_id, top_k, nearest_entities);
    output << entity_id_names_[e_id] << ":" << e_id << " ";
    for (int rank = 0; rank < top_k; ++rank) {
      // TODO
      int occur_cnt = 0;
      if (train_data->positive_entities(e_id).find(nearest_entities[rank].first)
          != train_data->positive_entities(e_id).end()) {
        occur_cnt++;
      }
      if (train_data->positive_entities(nearest_entities[rank].first).find(e_id)
          != train_data->positive_entities(nearest_entities[rank].first).end()) {
        occur_cnt++;
      }

      output << entity_id_names_[nearest_entities[rank].first] << ":" 
          << nearest_entities[rank].first << ":" 
          << nearest_entities[rank].second.first << ":" << occur_cnt << ":" 
          << nearest_entities[rank].second.second << " ";
    }
    output << endl;
    counter++;
    if (counter % (candidate_entities.size() / 10 + 1) == 0) {
      std::cout << "." << std::flush;
    }
  }

  std::cout << std::endl;
  output.flush();
  output.close();
}

void Analyst::ComputeEntityNearestNeibors(const int entity, const int top_k, 
    vector<pair<int, pair<float, float> > >& nearest_entities) {
#ifdef OPENMP
  #pragma omp parallel for
#endif
  for (int e_id = 0; e_id < solver_->num_entity(); ++e_id) {
    if (e_id == entity) { continue; }
    Path* path = ee_engine_->entity_category_hierarchy()->
        FindPathBetweenEntities(entity, e_id);
    path->RefreshAggrDistMetric(solver_->categories());
    float dist = solver_->ComputeDist(entity, e_id, path);

#ifdef OPENMP
  #pragma omp critical
#endif
  {  
    nearest_entities.push_back(
        make_pair(e_id, make_pair(dist, path->scale_2())));
  }

    delete path;
  }

  // sort
  std::partial_sort(nearest_entities.begin(), nearest_entities.begin() + top_k, 
      nearest_entities.end(), DesSortIntPairPairs());
}


} // namespace entity
