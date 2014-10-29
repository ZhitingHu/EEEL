// Author: Zhi-Ting Hu, Po-Yao Huang
// Date: 2014.10.26
#include <random>
#include <algorithm>
#include <cstdint>
#include <cmath>
#include <iostream>

#include "solver.hpp"
#include "util.hpp"
#include "string_util.hpp"
#include "context.hpp"

namespace entity {
	
namespace {

  int MyRandom(int i) {
  	static std::default_random_engine e;
  	return e() % i;
  }
  // Random init params to [-kRandInitRange, kRandInitRange].
  const float kRandInitRange = 0.05;
  
}   // anonymous namespace
	
Solver::Solver(const int num_entity, const int num_category) : 
  num_entity_(num_entity), num_category_(num_category) {
  // TODO initilaze from context flags
}

Solver::~Solver() {
  std::vector<Blob*>().swap(entities_);
  std::vector<Blob*>().swap(categories_);
}
	
// RandInit()
void Solver::RandInit() {
	std::random_device rd;
	std::mt19937 rng_engine(rd());
	std::uniform_real_distribution<float>
  dis(-kRandInitRange, kRandInitRange);

  // memory allocation
  std::vector<Blob*>(num_entity_, 0).swap(entities_);
  std::vector<Blob*>(num_category_, 0).swap(categories_);

  for (int i = 0; i < num_entity_; ++i)
      entities_[i] = new Blob(dim_entity_vector_);

  // Q: what's your plan to do with the Blob category? is that Mij? <= dim?
  for (int i = 0; i < num_category_; ++i)
      categories_[i] = new Blob(dim_entity_vector_);

  // Random init ei
  for (int i = 0; i < num_entity_; ++i)
      for (int j = 0; j < dim_entity_vector_; ++j)
          entities_[i]->set_data(j, static_cast <float> (rand()) / static_cast <float> (RAND_MAX));

  // Random init categories
  for (int i = 0; i < num_category_; ++i)
      for (int j = 0; j < dim_entity_vector_; ++j)
          categories_[i]->set_data(j, static_cast <float> (rand()) / static_cast <float> (RAND_MAX));


  M_diag_ = new float**[num_category_];
  for (int i = 0; i < num_category_; ++i){
      M_diag_[i] = new float*[num_entity_];
      for (int j = 0; j < num_entity_; ++j)
          M_diag_[i][j] = new float[1];
          //M_diag_[i][j] = new float[num_entity_];
  }

  // Random init Mcij (M_diag)
  for (int i = 0; i < num_category_; ++i)
      for (int j = 0; j < num_entity_; ++j)
          for (int k = 0; k < 1; ++k)
              M_diag_[i][j][k] = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
          //for (int k = 0; k < num_entity_; ++k)

  /*
  // memory allocation
  e_ = new float*[dim_entity_vector_];
  for (int i = 0; i < dim_entity_vector_; ++i)
  e_[i] = new float[num_entity_];

  m_ = new float*[num_entity_];
  for (int i = 0; i < num_entity_; ++i)
  m_[i] = new float[num_entity_];

  // random initialization
  for (int i = 0; i < dim_entity_vector_; ++i)
  for (int j = 0; j < num_entity_; ++j)
  e_[i][j] = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);


  if (distance_metric_mode_ == 0){
  for (int i = 0; i < num_entity_; ++i)
  for (int j = 0; j < num_entity_; ++j)
  m_[i][j] = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
  }
  else{

  std::cout << "not handled yet!";
  }*/


}// rnadinit()


float Solver::ComputeLikelihood(Blob* entity, float ***M_diag){
  float *v = new float[dim_entity_vector_];
  int *neg_set = new int[num_neg_sample_];
  float output_x_i = 0;
  const float *entity1, *entity2;

  for (int i = 0; i < num_entity_; ++i){
    //x = 100
    entity1 = entity->data();
    entity2 = entities_[i]->data();

    // v_ei - v_eo
    for (int k = 0; k < dim_entity_vector_; ++k)
        v[k] = entity1[k] - entity2[k];

    // v_ei' M v_ei
    // f_diagonals 1 => M is currently too big to initialize, temporarily using diagonals
    float *f_diagonals = new float[num_category_];
    for (int k = 0; k < dim_entity_vector_; ++k)
        f_diagonals[k] = 1;
    float acc = 0;
    for (int k = 0; k < dim_entity_vector_; ++k){
        //acc_neg_samp += v[k] * M_diag[class][1][j] * v[k];
        acc += v[k] * f_diagonals[k] * v[k];
    }
    delete f_diagonals;

    output_x_i = log(1 / (1 + exp(acc)));

    // negative sample set
    for (int j = 0; j < num_neg_sample_; ++j){
        neg_set[j] = static_cast <int> (rand()) / static_cast <int> (dim_entity_vector_);
    }

    acc = 0;
    for (int j = 0; j < num_neg_sample_; ++j){
      entity2 = entities_[neg_set[j]]->data();
      for (int k = 0; k < dim_entity_vector_; ++k)
          v[k] = entity1[k] - entity2[k];

      // trace M_ij
      // v_ei' M v_ei
      // f_diagonals = 1 => M is currently too big to initialize, temporarily using diagonals
      float *f_diagonals = new float[num_category_];
      for (int k = 0; k < dim_entity_vector_; ++k)
          f_diagonals[k] = 1;

      float acc_neg_samp = 0;
      for (int k = 0; k < dim_entity_vector_; ++k){
          //acc_neg_samp += v[k] * M_diag[class][1][j] * v[k];
          acc_neg_samp += v[k] * f_diagonals[k] * v[k];
      }
      acc += log(1 / (1 + exp(acc_neg_samp)));
      delete f_diagonals;
    }
    output_x_i += acc;
  }
  return output_x_i;
}


float Solver::ComputeDist(const int entity_from, const int entity_to, 
    const Path* path) {
  float* entity_from_vec = entities[entity_from]->data();
  float* entity_to_vec = entities[entity_to]->data();
  // xMx = sum_ij { x_i * x_j * M_ij }
  float dist = 0;
  if (dist_metric_mode_ == DistMetricMode::FULL) {
     Blob* dist_metric = path->aggr_dist_metric();
     for (int i = 0; i < dim_embedding_; ++i) {
       for (int j = 0; j < dim_embedding_; ++j) { 
         dist += entity_from_vec[i] * entity_to_vec[j] 
             * dist_metric->data_at(i, j);  
       }
     }
  } else if (dist_metric_mode_ == DistMetricMode::DIAG) {
      float* dist_metric_mat = path->aggr_dist_metric()->data();
      for (int i = 0; i < dim_embedding_; ++i) {
        dist += entity_from_vec[i] * entity_to_vec[i] * dist_metric_mat[i];  
      }
  } else if (dist_metric_mode_ == DistMetricMode::EDIAG) {
    //TODO
  } else {
    //TODO: report error
  }
  return dist;
}

void Solver::Solve(const vector<Datum*>& minibatch){
  //TODO: openmp parallelize
  for (int d = 0; d < minibatch.size(); ++d) {
    Datum* datum = minibatch[d];
    // Negatvie sampling
    //TODO

    // Computing Gradient
    // e_i

    // e_o  
 
    // neg_samples

    // Update
    //TODO
  }
}



}// namespace entity




/*
void Solver::Solve(){

float *v = new float[dim_entity_vector_];
int *set_neg = new int[num_neg_sample_];
float output_o_i = 0;
//f(eO | eI) => use for in solver or write an indep func

int o = 100;
//e_[i] - e_[j];
for (int i = 0; i < num_entity_; ++i){

// v_ei - v_eo
for (int k = 0; k < dim_entity_vector_; ++k){
v[k] = e_[k][i] - e_[k][i];
}

// v_ei' M v_ei
// f(i,o) = 1
float f_io = 1;
float temp = 0;
for (int k = 0; k < dim_entity_vector_; ++k){
temp += v[k] * f_io * v[k];
}
output_o_i = log(1 / (1 + exp(temp)));

// negative sample set
for (int j = 0; j < num_neg_sample_; ++j){
set_neg[j] = static_cast <int> (rand()) / static_cast <int> (dim_entity_vector_);
}

// compute //remark: merge with sampling later
float temp2 = 0;
for (int j = 0; j < num_neg_sample_; ++j){
// compute v_
for (int k = 0; k < dim_entity_vector_; ++k){
v[k] = e_[k][i] - e_[k][o];
}
// trace M_ij
// v_ei' M v_ei
// f(i,o) = 1
float f_io = 1;
float temp3 = 0;
for (int k = 0; k < dim_entity_vector_; ++k){
temp3 += v[k] * f_io * v[k];
}
temp2 += log(1 / (1 + exp(temp3)));
}
output_o_i -= temp2;

}

*/
