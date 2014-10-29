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
  void Solve(const vector<Datum*>& minibatch);

  float ComputeObjective(); // same as Likelihood();
  
  const int num_entity() { return num_entity_; }
  const int dim_embedding() { return dim_embedding_; }
  const int num_category() { return num_category_; }

private:
  void ComputeGradient();
  void Update();
  void Test();   
  //objective func P(y|x,M)
  float ComputeLikelihood(Blob* entity, float ***M_diag);
  float ComputeDist(const int entity_from, const int entity_to, 
      const Path* path);
  // grad += coeff * { (dist_metric + dist_metric^T) 
  //   * (entity_from_vec - entity_to_vec) } 
  void AccumulateGradient(const float coeff, const Blob* dist_metric, 
      const int entity_from, const int entity_to, Blob* grad);

private:
  vector<Blob*> entities_;
  vector<Blob*> categories_;

  int num_entity_;
  int num_category_;

  int num_neg_sample_;
  int dim_embedding_;
  double learning_rate_;
  entity::DistMetricMode dist_metric_mode_;
};

}  // namespace entity
