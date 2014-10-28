// Author: Zhi-Ting Hu, Po-Yao Huang
// Date: 2014.10.26
#pragma once
#include "blob.hpp"
#include "dataset.hpp"
#include <cstdint>
#include <vector>
using namespace std;

namespace entity {

class Solver {
public:
  Solver(int32_t num_entity, int32_t num_cat, int32_t num_ns, int32_t num_data, 
    Dataset* dataset, int32_t dim, int32_t mode, double rate);
  ~Solver();
  
  // Initialization
  //void Init(); => merge into constructore
  void RandInit();
  void Solve(const vector<datum*>& minibatch);

  float ComputeObjective(); // same as Likelihood();
  
  const int num_entity() { return num_entity_; }
  const int dim_entity_vector() { return dim_entity_vector_; }
  const int num_category() { return num_category_; }
  const int dim_category_matrix() { return dim_entity_vector_; }

private:
  void ComputeGradient();
  void Update();
  void Test();  
  float Likelihood(Blob* entity, float ***M_diag); //objective func P(y|x,M)

private:
  // e_[i]: entity_i 's vector
  //float** e_;
	  
  // not sure what's your plan with categories and M, reserve Mij first and simplify it to diag
  float*** M_diag_;

  vector<Blob*> entities_;
  vector<Blob*> categories_;

  const Dataset *dataset_;  // data passed from parsing inputs TODO

  // const parameters
  const int num_entity_;
  const int num_category_;
  const int num_neg_sample_;
  const int num_data_;
  const int dim_entity_vector_;
  const int distance_metric_mode_;
  const double learning_rate_;
	  
};

}  // namespace entity
