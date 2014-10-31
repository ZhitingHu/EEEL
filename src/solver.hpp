// Author: Zhi-Ting Hu, Po-Yao Huang
// Date: 2014.10.26
#pragma once
#include "blob.hpp"
#include "dataset.hpp"
#include <cstdint>
#include <vector>

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

  const float ComputeObjective();
  
  const int num_entity() { return num_entity_; }
  const int dim_embedding() { return dim_embedding_; }
  const int num_category() { return num_category_; }

private:
  void ComputeGradient();
  void Update();
  void Test();   
  float ComputeDist(const int entity_from, const int entity_to, 
      const Path* path);
  // grad += coeff * { (dist_metric + dist_metric^T) 
  //   * (entity_from_vec - entity_to_vec) } 
  // Note: the ordering of entity_from and entity_to matters!
  void AccumulateEntityGradient(const float coeff, const Blob* dist_metric, 
      const int entity_from, const int entity_to, Blob* grad);
  // grad += coeff * (entity_from_vec - entity_to_vec)^2 
  void AccumulateCateogryGradient(const float coeff, const int entity_from, 
    const int entity_to, Blob* grad);
  void SampleNegEntities(const Datum* datum);
  void ComputeEntityGradient(const Datum* datum);
  void ComputeCategoryGradient(const Datum* datum);

private:
  vector<Blob*> entities_;
  vector<Blob*> categories_;

  int num_entity_;
  int num_category_;

  int num_neg_sample_;
  int dim_embedding_;
  double learning_rate_; //TODO adaptive lr
  int num_iter_on_entity_;
  int num_iter_on_category_;
  entity::DistMetricMode dist_metric_mode_;
};

}  // namespace entity
