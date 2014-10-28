#pragma once

#include "blob.hpp"

#include <cstdint>
#include <vector>

namespace entity {

class Solver {
public:
  Solver();
  ~Solver();
  
  // Initialization
  void Init();

  void Solve();

  float ComputeObjective();
  
  const int num_entity() { return num_entity_; }
  const int dim_entity_vector() { return dim_entity_vector_; }
  const int num_category() { return num_category_; }
  const int dim_category_matrix() { return dim_entity_vector_; }

private:
  void ComputeGradient();
  void Update();

  void Test();

private:
  vector<Blob*> entities_;
  vector<Blob*> categories_;

  //
  const int num_entity_;
  const int dim_entity_vector_;
  const int num_category_;
};

}  // namespace dnn
