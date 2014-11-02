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
  // (zhiting hu): cannot compile under -std=c++0x 

  //int MyRandom(int i) {
  //	static std::default_random_engine e;
  //	return e() % i;
  //}
  // Random init params to [-kRandInitRange, kRandInitRange].
  const float kRandInitRange = 0.05;
  
}   // anonymous namespace
	
Solver::Solver(const int num_entity, const int num_category) : 
    num_entity_(num_entity), num_category_(num_category) {
  entity::Context& context = entity::Context::get_instance();
  num_neg_sample_ = context.get_int32("num_neg_sample");
  learning_rate_ = context.get_double("learning_rate"); //TODO adaptive lr 
  num_epoch_on_batch_ = context.get_int32("num_epoch_on_batch");
  num_iter_on_entity_ = context.get_int32("num_iter_on_entity");
  num_iter_on_category_ = context.get_int32("num_iter_on_category");
  openmp_ = context.get_bool("openmp");
}

Solver::~Solver() {
  std::vector<Blob*>().swap(entities_);
  std::vector<Blob*>().swap(categories_);
}
	
void Solver::RandInit() {
  std::random_device rd;
  std::mt19937 rng_engine(rd());
  // (zhiting hu) cannot compile under -std=c++0x
  //std::uniform_real_distribution<float> dis(-kRandInitRange, kRandInitRange);

  for (int i = 0; i < num_entity_; ++i) {
    entities_.push_back(new Blob(entity::Context::dim_embedding()));
  }
  for (int i = 0; i < num_category_; ++i) {
    categories_.push_back(new Blob(
        entity::Context::dim_embedding(), entity::Context::dim_embedding()));
  }

  for (int i = 0; i < num_entity_; ++i) {
    for (int j = 0; j < entity::Context::dim_embedding(); ++j) {
      entities_[i]->mutable_data()[j] 
          = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    }
  }
  for (int c_idx = 0; c_idx < num_category_; ++c_idx) {
    for (int i = 0; i < entity::Context::dim_embedding(); ++i) {
      for (int j = 0; j < entity::Context::dim_embedding(); ++j) {
        categories_[c_idx]->init_data_at(
            static_cast <float> (rand()) / static_cast <float> (RAND_MAX), i, j);
      }
    }
  }
}

const float Solver::ComputeObjective(const vector<Datum*>& val_batch) {
  if (openmp_) {
    return ComputeObjective_omp(val_batch);
  } else {
    return ComputeObjective_single(val_batch);
  }
}

const float Solver::ComputeObjective_single(const vector<Datum*>& val_batch) {
  float obj = 0;
  for (int d = 0; d < val_batch.size(); ++d) {
    Datum* datum = val_batch[d];

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

const float Solver::ComputeObjective_omp(const vector<Datum*>& val_batch) {
  LOG(FATAL) << "Do not support openmp.";
  float obj = 0;
  //TODO openmp parallelize
  for (int d = 0; d < val_batch.size(); ++d) {
    Datum* datum = val_batch[d];

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

const float Solver::ComputeDist(const int entity_from, const int entity_to, 
    const Path* path) {
  const float* entity_from_vec = entities_[entity_from]->data();
  const float* entity_to_vec = entities_[entity_to]->data();
  // xMx = sum_ij { x_i * x_j * M_ij }
  float dist = 0;
  if (entity::Context::dist_metric_mode() == entity::Context::DIAG) {
    const float* dist_metric_mat = path->aggr_dist_metric()->data();
    for (int i = 0; i < entity::Context::dim_embedding(); ++i) {
      dist += entity_from_vec[i] * entity_to_vec[i] * dist_metric_mat[i];  
    }
  } else if (entity::Context::dist_metric_mode() == entity::Context::EDIAG) {
    LOG(FATAL) << "do not support DistMetricMode::EDIAG right now.";
  } else if (entity::Context::dist_metric_mode() == entity::Context::FULL) {
     const Blob* dist_metric = path->aggr_dist_metric();
     for (int i = 0; i < entity::Context::dim_embedding(); ++i) {
       for (int j = 0; j < entity::Context::dim_embedding(); ++j) { 
         dist += entity_from_vec[i] * entity_to_vec[j] 
             * dist_metric->data_at(i, j);  
       }
     }
  } else {
    LOG(FATAL) << "Unkown Distance_Metric_Mode";
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
  const float* entity_from_vec = entities_[entity_from]->data();
  const float* entity_to_vec = entities_[entity_to]->data();
  if (entity::Context::dist_metric_mode() == entity::Context::DIAG) {
    const float* dist_metric_mat = dist_metric->data();
    for (int i = 0; i < entity::Context::dim_embedding(); ++i) {
      grad_vec[i] += coeff * (2 * dist_metric_mat[i]) 
          * (entity_from_vec[i] - entity_to_vec[i]);
    }
  } else if (entity::Context::dist_metric_mode() == entity::Context::EDIAG) {
    LOG(FATAL) << "do not support DistMetricMode::EDIAG right now.";
  } else if (entity::Context::dist_metric_mode() == entity::Context::FULL) {
    LOG(FATAL) << "do not support DistMetricMode::FULL right now.";
  } else {
    LOG(FATAL) << "Unkown Distance_Metric_Mode";
  }
}

void Solver::AccumulateCategoryGradient(const float coeff, 
    const int entity_from, const int entity_to, Blob* grad) {
  float* grad_mat = grad->mutable_data();
  const float* entity_from_vec = entities_[entity_from]->data();
  const float* entity_to_vec = entities_[entity_to]->data();
  if (entity::Context::dist_metric_mode() == entity::Context::DIAG) {
    for (int i = 0; i < entity::Context::dim_embedding(); ++i) {
      grad_mat[i] += coeff * (entity_from_vec[i] - entity_to_vec[i])
          * (entity_from_vec[i] - entity_to_vec[i]);;
    }
  } else if (entity::Context::dist_metric_mode() == entity::Context::EDIAG) {
    LOG(FATAL) << "do not support DistMetricMode::EDIAG right now.";
  } else if (entity::Context::dist_metric_mode() == entity::Context::FULL) {
    LOG(FATAL) << "do not support DistMetricMode::FULL right now.";
  } else {
    LOG(FATAL) << "Unkown Distance_Metric_Mode";
  }
}


void Solver::ComputeEntityGradient(Datum* datum) {
    // on e_i
    const int entity_i = datum->entity_i();
    Blob* entity_i_grad = datum->entity_i_grad();
    float coeff = 1.0 - fastsigmoid(ComputeDist(
        entity_i, datum->entity_o(), datum->category_path()));
    AccumulateEntityGradient(coeff, datum->category_path()->aggr_dist_metric(), 
        entity_i, datum->entity_o(), entity_i_grad);
    // on e_o  
    // = (-1) * gradient_on_e_i, so simply do the copy
    datum->entity_o_grad()->CopyFrom(entity_i_grad, -1.0);

    // neg_samples
    for (int neg_idx = 0; neg_idx < num_neg_sample_; ++neg_idx) {
      Blob* neg_entity_grad = datum->neg_entity_grad(neg_idx);
      coeff = 1.0 - fastsigmoid(-1.0 * ComputeDist(
          entity_i, datum->neg_entity(neg_idx), datum->category_path()));
      AccumulateEntityGradient(
          coeff, datum->neg_category_path(neg_idx)->aggr_dist_metric(), 
          entity_i, datum->neg_entity(neg_idx), neg_entity_grad);
      // Accumulate (-1) * gradient_on_neg_samples to gradient_on_e_i 
      entity_i_grad->Accumulate(neg_entity_grad, -1.0);
    }
}

void Solver::ComputeCategoryGradient(Datum* datum) {
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
  const vector<Path*>& neg_category_paths = datum->neg_category_paths();
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
  if (openmp_) {
    Solve_omp(minibatch);
  } else {
    Solve_single(minibatch);
  }
}

void Solver::Solve_single(const vector<Datum*>& minibatch) {
#ifdef DEBUG
  CHECK(minibatch.size() != 0);
#endif
  float update_coeff = (-1.0) * learning_rate_ / minibatch.size();
  for (int epoch = 0; epoch < num_epoch_on_batch_; ++epoch) {
    // Optimize entity embedding
    for (int d = 0; d < minibatch.size(); ++d) {
      Datum* datum = minibatch[d];
      //TODO Negative sampling
      //SampleNegEntities(datum);

      for (int iter = 0; iter < num_iter_on_entity_; ++iter) {
        ComputeEntityGradient(datum);

        //Update Entity Vectors
        entities_[datum->entity_i()]->Accumulate(
            datum->entity_i_grad(), update_coeff);
        entities_[datum->entity_o()]->Accumulate(
            datum->entity_o_grad(), update_coeff);
        for (int neg_idx = 0; neg_idx < num_neg_sample_; ++neg_idx) {
          entities_[datum->neg_entity(neg_idx)]->Accumulate(
              datum->neg_entity_grad(neg_idx), update_coeff);
        }
        //Projection
        entities_[datum->entity_i()]->Normalize();
        entities_[datum->entity_o()]->Normalize();
        for (int neg_idx = 0; neg_idx < num_neg_sample_; ++neg_idx) {
          entities_[datum->neg_entity(neg_idx)]->Normalize();
        }
      }
    }
    // Optimize category embedding
    for (int d = 0; d < minibatch.size(); ++d) {
      Datum* datum = minibatch[d];
      
      const map<int, int>& category_index = datum->category_index();
      map<int, int>::const_iterator it = category_index.begin();
      const vector<Blob*>& category_grads = datum->category_grads();
      for (int iter = 0; iter < num_iter_on_category_; ++iter) {
        ComputeCategoryGradient(datum); 

        //Update Category Matrics
        for (it = category_index.begin(); it != category_index.end(); ++it) {
          categories_[it->first]->Accumulate(
              category_grads[it->second], update_coeff);
        }
        // Refresh path aggregated distance metric
        datum->category_path()->RefreshAggrDistMetric(categories_);
        vector<Path*> neg_category_paths = datum->neg_category_paths();
        for (int p_idx = 0; p_idx < neg_category_paths.size(); ++p_idx) {
          neg_category_paths[p_idx]->RefreshAggrDistMetric(categories_);
        } 
      }
    }
  }// end of epoches
}

void Solver::Solve_omp(const vector<Datum*>& minibatch) {
  LOG(FATAL) << "Do not support opemmp.";
#ifdef DEBUG
  CHECK(minibatch.size() != 0);
#endif
  float update_coeff = (-1.0) * learning_rate_ / minibatch.size();
  for (int epoch = 0; epoch < num_epoch_on_batch_; ++epoch) {
    //TODO: openmp parallelize
    for (int d = 0; d < minibatch.size(); ++d) {
      Datum* datum = minibatch[d];
      //TODO Negative sampling
      //SampleNegEntities(datum);

      for (int iter = 0; iter < num_iter_on_entity_; ++iter) {
        ComputeEntityGradient(datum);

        //Update Entity Vectors
        entities_[datum->entity_i()]->Accumulate(
            datum->entity_i_grad(), update_coeff);
        entities_[datum->entity_o()]->Accumulate(
            datum->entity_o_grad(), update_coeff);
        for (int neg_idx = 0; neg_idx < num_neg_sample_; ++neg_idx) {
          entities_[datum->neg_entity(neg_idx)]->Accumulate(
              datum->neg_entity_grad(neg_idx), update_coeff);
        }
        //Projection
        entities_[datum->entity_i()]->Normalize();
        entities_[datum->entity_o()]->Normalize();
        for (int neg_idx = 0; neg_idx < num_neg_sample_; ++neg_idx) {
          entities_[datum->neg_entity(neg_idx)]->Normalize();
        }
      }
    }

    /// openmp sync here ..
    
    //TODO: openmp parallelize
    for (int d = 0; d < minibatch.size(); ++d) {
      Datum* datum = minibatch[d];
      
      const map<int, int>& category_index = datum->category_index();
      map<int, int>::const_iterator it = category_index.begin();
      const vector<Blob*>& category_grads = datum->category_grads();
      for (int iter = 0; iter < num_iter_on_category_; ++iter) {
        ComputeCategoryGradient(datum); 

        //Update Category Matrics
        for (it = category_index.begin(); it != category_index.end(); ++it) {
          categories_[it->first]->Accumulate(
              category_grads[it->second], update_coeff);
        }
        // Refresh path aggregated distance metric
        datum->category_path()->RefreshAggrDistMetric(categories_);
        vector<Path*> neg_category_paths = datum->neg_category_paths();
        for (int p_idx = 0; p_idx < neg_category_paths.size(); ++p_idx) {
          neg_category_paths[p_idx]->RefreshAggrDistMetric(categories_);
        } 
      }
    }
  } // end of epoches
}

} // namespace entity
