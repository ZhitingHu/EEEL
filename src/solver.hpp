// Author: Zhi-Ting Hu, Po-Yao Huang
// Date: 2014.10.26
#ifndef ENTITY_SOLVER_HPP_
#define ENTITY_SOLVER_HPP_

#include "blob.hpp"
#include "datum.hpp"
#include <cstdint>
#include <vector>
#include <set>

namespace entity {

class Solver {
public:
  Solver(const int num_entity, const int num_category);
  ~Solver();
  
  // Initialization
  //void Init(); => merge into constructore
  void RandInit();

  // Optimizes based on minibatch
  // Assumes symmetric distance, i.e. d(e_i, e_o) = d(e_o, e_i)
  void Solve(const vector<Datum*>& minibatch);
  
  void Solve_single(const vector<Datum*>& minibatch);
  void Solve_omp(const vector<Datum*>& minibatch);

  const float ComputeObjective(const vector<Datum*>& val_batch);
  const float ComputeObjective_single(const vector<Datum*>& val_batch);
  const float ComputeObjective_omp(const vector<Datum*>& val_batch);
  
  const int num_entity() { return num_entity_; }
  const int dim_embedding() { return dim_embedding_; }
  const int num_category() { return num_category_; }

private:
  const float ComputeDist(const int entity_from, const int entity_to, 
      const Path* path);

  // grad += coeff * { (dist_metric + dist_metric^T) 
  //   * (entity_from_vec - entity_to_vec) } 
  // Note: the ordering of entity_from and entity_to matters!
  void AccumulateEntityGradient(const float coeff, const Blob* dist_metric, 
      const int entity_from, const int entity_to, Blob* grad);
  // grad += coeff * (entity_from_vec - entity_to_vec)^2 
  void AccumulateCategoryGradient(const float coeff, const int entity_from, 
    const int entity_to, Blob* grad);

  void ComputeEntityGradient(Datum* datum);
  void ComputeCategoryGradient(Datum* datum);


private:
  vector<Blob*> entities_;
  vector<Blob*> categories_;
  
  // used in single-thread version
  vector<Blob*> entity_grads_;
  vector<Blob*> category_grads_;
  set<int> updated_entities_;
  set<int> updated_categories_;
  set<int>::const_iterator set_it_;
  map<int, int>::const_iterator map_it_;
  
  int num_entity_;
  int num_category_;

  int num_neg_sample_;
  int dim_embedding_;
  double learning_rate_; //TODO adaptive lr
  int num_epoch_on_batch_;
  int num_iter_on_entity_;
  int num_iter_on_category_;
  bool openmp_;
};

}  // namespace entity

#endif
