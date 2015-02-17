#ifndef ENTITY_ANALYST_HPP_
#define ENTITY_ANALYST_HPP_

#include "solver.hpp"
#include "ee_engine.hpp"
#include <vector>
#include <set>

namespace entity {

class Analyst {
public:
  Analyst(const string& snapshot_path, const int iter);
  ~Analyst() { }
 
  void ReadIdNames(const string& filename, map<int, string>& id_names);

  void ComputeNearestNeiborsByCategories(
    const int top_k, const vector<int>& candidate_entities,
    const vector<vector<vector<int> > >& categories, const string& output_path);

  void ComputeEntityNearestNeiborsByCategories(
    const int entity, const int top_k, const vector<int>& categories,
    vector<pair<int, float> >& nearest_entities);

  void ComputeNearestNeibors(const int top_k, 
      const vector<int>& candidate_entities, const string& output_filename);

  void ComputeEntityNearestNeibors(const int entity, const int top_k, 
      vector<pair<int, pair<float, float> > >& nearest_entities);
 
private:
  EEEngine* ee_engine_;
  Solver* solver_;

  map<int, string> entity_id_names_;
  map<int, string> category_id_names_;
};

}  // namespace entity

#endif
