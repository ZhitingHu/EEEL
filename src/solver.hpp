#pragma once

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
  // e_[i]: entity_i 's vector
  float** e_;
  // m_[i]: distance metric of category i
  float*** m_;

  //
  const int num_entity_;
  const int dim_entity_vector_;
  const int num_category_;
};

}  // namespace dnn
