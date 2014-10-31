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
#include "fastapprox/fastsigmoid.h"

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
  std::uniform_real_distribution<float> dis(-kRandInitRange, kRandInitRange);

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


const float Solver::ComputeObjective(vector<Datum*> val_batch) {
  float obj = 0;
  //TODO openmp parallelize
  for (int d = 0; d < minibatch.size(); ++d) {
    Datum* datum = minibatch[d];

    //TODO negative sampling

    float datum_obj = 0;
    int entity_i = datum->entity_i();
    datum_obj += log(fastsigmoid(ComputeDist(
        entity_i, datum->entity_o(), datum->category_path())));
    for (int neg_idx = 0; neg_idx < num_neg_sample_; ++neg_idx) {
      datum_obj += log(fastsigmoid((-1.0) * ComputeDist(
          entity_i, datum->neg_entity(neg_idx), datum->category_path())));
    }
    obj += datum_obj;
  } 
  return obj; 
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

void Solver::AccumulateEntityGradient(const float coeff, 
    const Blob* dist_metric, const int entity_from, const int entity_to, 
    Blob* grad) {
#ifdef DEBUG
  CHECK(dist_metric);
  CHECK(grad);
#endif
  float* grad_vec = grad->mutable_data();
  float* entity_from_vec = entities[entity_from]->data();
  float* entity_to_vec = entities[entity_to]->data();
  if (dist_metric_mode_ == DistMetricMode::FULL) {
    LOG(FATAL) << "do not support DistMetricMode::FULL right now.";
  } else if (dist_metric_mode_ == DistMetricMode::DIAG) {
    float* dist_metric_mat = path->aggr_dist_metric()->data();
    for (int i = 0; i < dim_embedding_; ++i) {
      grad_vec[i] += coeff * (2 * dist_metric_mat[i]) 
          * (entity_from_vec[i] - entity_to_vec[i]);
    }
  } else if (dist_metric_mode_ == DistMetricMode::EDIAG) {
    LOG(FATAL) << "do not support DistMetricMode::EDIAG right now.";
  } else {
    LOG(FATAL) << "Unkown DistMetricMode";
  }
}

void Solver::AccumulateCategoryGradient(const float coeff, 
    const int entity_from, const int entity_to, Blob* grad) {
  float* grad_mat = grad->mutable_data();
  float* entity_from_vec = entities[entity_from]->data();
  float* entity_to_vec = entities[entity_to]->data();
  if (dist_metric_mode_ == DistMetricMode::FULL) {
    LOG(FATAL) << "do not support DistMetricMode::FULL right now.";
  } else if (dist_metric_mode_ == DistMetricMode::DIAG) {
    float* dist_metric_mat = path->aggr_dist_metric()->data();
    for (int i = 0; i < dim_embedding_; ++i) {
      grad_mat[i] += coeff * (entity_from_vec[i] - entity_to_vec[i])
          * (entity_from_vec[i] - entity_to_vec[i]);;
    }
  } else if (dist_metric_mode_ == DistMetricMode::EDIAG) {
    LOG(FATAL) << "do not support DistMetricMode::EDIAG right now.";
  } else {
    LOG(FATAL) << "Unkown DistMetricMode";
  }
}

void Solver::SampleNegEntities(const Datum* datum) {
  //TODO
}

void Solver::AddNegSample(const int neg_entity_id, const Path* path) {
  //TODO
}


void Solver::ComputeEntityGradient(const Datum* datum) {
    // on e_i
    const int entity_i = datum->entity_i();
    Blob* entity_i_grad = datum->entity_i_grad();
    float coeff = 1.0 - fastsigmoid(ComputeDist(
        entity_i, datum->entity_o(), datum->category_path()));
    AccumulateGradient(coeff, datum->category_path()->aggr_dist_metric(), 
        entity_i, datum->entity_o(), entity_i_grad);
    // on e_o  
    // = (-1) * gradient_on_e_i, so simply do the copy
    datum->entity_o_grad()->CopyFrom(entity_i_grad, -1.0);

    // neg_samples
    for (int neg_idx = 0; neg_idx < num_neg_sample_; ++neg_idx) {
      Blob* neg_entity_grad = datum->neg_entity_grad(neg_idx);
      coeff = 1.0 - fastsigmoid(-1.0 * ComputeDist(
          entity_i, datum->neg_entity(neg_idx), datum->category_path()));
      AccumulateGradient(
          coeff, datum->neg_category_path(neg_idx)->aggr_dist_metric(), 
          entity_i, datum->neg_entity(neg_idx), neg_entity_grad);
      // Accumulate (-1) * gradient_on_neg_samples to gradient_on_e_i 
      entity_i_grad->Accumulate(neg_entity_grad, -1.0);
    }
}

void Solver::ComputeCategoryGradient(const Datum* datum) {
  // process (e_i,  e_o)
  const vector<int>& category_nodes = datum->category_path()->category_nodes();
  const int entity_i = datum->entity_i();
  const int entity_o = datum->entity_o();
  float coeff = 1.0 - fastsigmoid(ComputeDist(
      entity_i, entity_o, datum->category_path()));
  for (int c_idx = 0; c_idx < category_nodes.size(); ++c_idx) {
    AccumulateCategoryGradient(coeff, entity_i, entity_o, 
        datum->category_grad(category_nodes[c_idx]));
  }

  // process (e_i, negative samples)
  vector<Path*>& neg_category_paths = datum->neg_category_paths();
  for (int path_idx = 0; path_idx < neg_category_paths.size(); ++path_idx) {
    const vector<int>& neg_category_nodes 
        = neg_category_paths[path_idx]->category_nodes();
    const int neg_entity = datum->neg_entity(path_idx);
    float coeff = fastsigmoid(-1.0 * ComputeDist(
        entity_i, neg_entity, neg_category_paths[path_idx])) - 1.0;
    for (int c_idx = 0; c_idx < neg_category_nodes.size(); ++c_idx) {
      AccumulateCategoryGradient(coeff, entity_i, neg_entity, 
          datum->category_grad(neg_category_nodes[c_idx]));
    }
  }
}

void Solver::Solve(const vector<Datum*>& minibatch) {
#ifdef DEBUG
  CHECK(minibatch.size() != 0);
#endif
  float update_coeff = (-1.0) * learning_rate_ / minibatch.size();
  //TODO: openmp parallelize
  for (int d = 0; d < minibatch.size(); ++d) {
    Datum* datum = minibatch[d];
    //TODO Negative sampling
    //SampleNegEntities(datum);

    for (int iter = 0; iter < num_iter_on_entity; ++iter) {
      ComputeEntityGradient(datum);

      //Update Entity Vectors
      entities[datum->entity_i()]->Accumulate(
          datum->entity_i_grad(), update_coeff);
      entities[datum->entity_o()]->Accumulate(
          datum->entity_o_grad(), update_coeff);
      for (int neg_idx = 0; neg_idx < num_neg_sample_; ++neg_idx) {
        entities[datum->neg_entity(neg_idx)]->Accumulate(
            datum->neg_entity_grad(neg_idx), update_coeff);
      }
      //TODO projection
    }
  }

  /// openmp sync here ..
  
  //TODO: openmp parallelize
  for (int d = 0; d < minibatch.size(); ++d) {
    Datum* datum = minibatch[d];
    
    const map<int, int>& category_index = datum->category_index();
    map<int, int>::iterator it = category_index.begin();
    const vector<Blob*>& category_grads = datum->category_grads();
    for (int iter = 0; iter < num_iter_on_category; ++iter) {
      ComputeCategoryGradient(datum); 

      //Update Category Matrics
      for (it = category_index.begin(); it != category_index.end(); ++it) {
        categories[it->first]->Accumulate(
            category_grads[it->second], update_coeff);
      }
    }
  }

}

} // namespace entity
