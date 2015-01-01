// Author: Zhi-Ting Hu, Po-Yao Huang
// Date: 2014.10.26
#include <random>
#include <algorithm>
#include <stdint.h>
#include <cmath>
#include <iostream>
#include <fstream>
#include <iterator>

#include "solver.hpp"
#include "util.hpp"
#include "string_util.hpp"
#include "context.hpp"
//#include "fastapprox/fastsigmoid.h"
#include "pl_math.h"

namespace entity {
	
namespace {
  // (zhiting hu): cannot compile under -std=c++0x 

  //int MyRandom(int i) {
  //	static std::default_random_engine e;
  //	return e() % i;
  //}
  // Random init params to [-kRandInitRange, kRandInitRange].
  const float kRandInitRange = 0.05;
  const float kEpsilon = 1e-20;
  
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

  if (!openmp_) {
    for (int i = 0; i < num_entity_; ++i) {
      entity_grads_.push_back(new Blob(entity::Context::dim_embedding()));
    }
    for (int i = 0; i < num_category_; ++i) {
      category_grads_.push_back(new Blob(
          entity::Context::dim_embedding(), entity::Context::dim_embedding()));
    }
  }
}

Solver::~Solver() {
  std::vector<Blob*>().swap(entities_);
  std::vector<Blob*>().swap(categories_);
}
	
void Solver::RandInit() {
  //std::random_device rd;
  //std::mt19937 rng_engine(rd());
  //// (zhiting hu) cannot compile under -std=c++0x
  //std::uniform_real_distribution<float> dis(-kRandInitRange, kRandInitRange);

  for (int i = 0; i < num_entity_; ++i) {
    entities_.push_back(new Blob(entity::Context::dim_embedding()));
  }
  for (int i = 0; i < num_category_; ++i) {
    categories_.push_back(new Blob(
        entity::Context::dim_embedding(), entity::Context::dim_embedding()));
  }

  for (int e_idx = 0; e_idx < num_entity_; ++e_idx) {
    for (int i = 0; i < entity::Context::dim_embedding(); ++i) {
      entities_[e_idx]->init_data_at(
          static_cast <float> (rand()) / static_cast <float> (RAND_MAX), i);
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

void Solver::Snapshot(const string& output_path, const int iter) {
  LOG(ERROR) << "Snapshoting to " << output_path << " iter=" << iter;

  ostringstream oss;
  oss << output_path << "/parameter_" << iter;
  SnapshotParameters(oss.str());

  oss.str("");
  oss.clear();
  oss << output_path << "/entity_vectors_" << iter;
  SnapshotBlobs(oss.str(), entities_);  

  oss.str("");
  oss.clear();
  oss << output_path << "/category_vectors_" << iter;
  SnapshotBlobs(oss.str(), categories_);  
}

void Solver::SnapshotParameters(const string& param_filename) {
  //TODO
}

/**
 * Output format: 
 *   line 1:  blob.count blob.num_row blob.num_col 
 *   line 2-: blob.data
 *
 **/
void Solver::SnapshotBlobs(const string& blobs_filename, const vector<Blob*>& blobs) {
#ifdef DEBUG
  CHECK_GT(blobs.size(), 0);
#endif
  ofstream blob_snapshot;
  blob_snapshot.open(blobs_filename.c_str());
  blob_snapshot << blobs[0]->count() << " " << blobs[0]->num_row() 
      << " " << blobs[0]->num_col() << " " << endl;
  for (int idx = 0; idx < blobs.size(); ++idx) {
    const float* blob_data = blobs[idx]->data();
    for (int d_idx = 0; d_idx < blobs[idx]->count(); ++d_idx) {
      blob_snapshot << blob_data[d_idx] << " ";
    }
    blob_snapshot << endl;
  }  
  blob_snapshot.close();
}

void Solver::Restore(const string& snapshot_path, const int iter) {
  LOG(ERROR) << "Restoring from "<< snapshot_path << " iter=" << iter;

  ostringstream oss;
  oss << snapshot_path << "/parameter_" << iter;
  RestoreParameters(oss.str());

  oss.str("");
  oss.clear();
  oss << snapshot_path << "/entity_vectors_" << iter;
  RestoreBlobs(oss.str(), entities_);  
  // temp, should be initialized in RestoreParameters()
  num_entity_ = entities_.size();
  LOG(ERROR) << "number of entities: " << num_entity_;   

  oss.str("");
  oss.clear();
  oss << snapshot_path << "/category_vectors_" << iter;
  RestoreBlobs(oss.str(), categories_);  
  // temp, should be initialized in RestoreParameters()
  num_category_ = categories_.size();
  LOG(ERROR) << "number of categories: " << num_category_;   
}

void Solver::RestoreParameters(const string& param_filename) {
  //TODO
}

void Solver::RestoreBlobs(const string& blobs_filename, vector<Blob*>& blobs) {
  LOG(ERROR) << "Restoring from " << blobs_filename;

  ifstream blobs_snapshot(blobs_filename);
  string line;
  // line 1
  getline(blobs_snapshot, line);
  istringstream header_iss(line);
  int count, num_row, num_col;
  header_iss >> count >> num_row >> num_col;
  LOG(ERROR) << count << " " << num_row << " " << num_col;

  // line 2-
  while (getline(blobs_snapshot, line)) {
    Blob* blob = new Blob(num_row, num_col);
    istringstream iss(line);
    vector<float> tokens((std::istream_iterator<float>(iss)), 
        std::istream_iterator<float>());

    for (int idx = 0; idx < tokens.size(); ++idx) {
      blob->mutable_data()[idx] = tokens[idx];
    }
    blobs.push_back(blob);
  }
  blobs_snapshot.close();
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
    datum->category_path()->RefreshAggrDistMetric(categories_);

    float datum_obj = 0;
    int entity_i = datum->entity_i();
    datum_obj += log(max(kEpsilon, PLearn::fastsigmoid((-1.0) * ComputeDist(
        entity_i, datum->entity_o(), datum->category_path()))));
#ifdef DEBUG
    if (isnan(datum_obj)) {
      LOG(ERROR) << (-1.0) * PLearn::fastsigmoid(ComputeDist(entity_i, datum->entity_o(), datum->category_path()));
    }
    CHECK(!isnan(datum_obj));
#endif
    for (int neg_idx = 0; neg_idx < num_neg_sample_; ++neg_idx) {
      datum_obj += log(max(kEpsilon, PLearn::fastsigmoid(ComputeDist(
          entity_i, datum->neg_entity(neg_idx), datum->category_path()))));
#ifdef DEBUG
    CHECK(!isnan(datum_obj));
#endif
    }
    float tmp = obj;
    obj += datum_obj;
#ifdef DEBUG
    if (isnan(obj)) {
      LOG(ERROR) << obj << " = " << tmp << " + " << datum_obj;
    }
    CHECK(!isnan(obj));
#endif
  } 
  return obj; 
}

const float Solver::ComputeObjective_omp(const vector<Datum*>& val_batch) {
  LOG(FATAL) << "Do not support openmp.";
  float obj = 0;
  //TODO openmp parallelize
  for (int d = 0; d < val_batch.size(); ++d) {
    Datum* datum = val_batch[d];
    datum->category_path()->RefreshAggrDistMetric(categories_);

    float datum_obj = 0;
    int entity_i = datum->entity_i();
    datum_obj += log(PLearn::fastsigmoid((-1.0) * ComputeDist(
        entity_i, datum->entity_o(), datum->category_path())));
    for (int neg_idx = 0; neg_idx < num_neg_sample_; ++neg_idx) {
      datum_obj += log(PLearn::fastsigmoid(ComputeDist(
          entity_i, datum->neg_entity(neg_idx), datum->category_path())));
    }
    obj += datum_obj;
  } 
  return obj; 
}

const float Solver::ComputeDist(const int entity_from, const int entity_to, 
    const Path* path) {
#ifdef DEBUG
  CHECK(path);
#endif
  const float* entity_from_vec = entities_[entity_from]->data();
#ifdef DEBUG
  CHECK(entity_from_vec);
#endif
  const float* entity_to_vec = entities_[entity_to]->data();
#ifdef DEBUG
  CHECK(entity_to_vec);
#endif
  // xMx = sum_ij { x_i * x_j * M_ij }
  float dist = 0;
  if (entity::Context::dist_metric_mode() == entity::Context::DIAG) {
#ifdef DEBUG
  CHECK(path->aggr_dist_metric());
#endif
    const float* dist_metric_mat = path->aggr_dist_metric()->data();
#ifdef DEBUG
  CHECK(dist_metric_mat);
#endif
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
#ifdef DEBUG
  CHECK(!isnan(dist));
#endif
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
#ifdef DEBUG
    CHECK(!isnan(grad_vec[i]));
#endif
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
    float coeff = PLearn::fastsigmoid((-1.0) * ComputeDist(
        entity_i, datum->entity_o(), datum->category_path())) - 1.0;
#ifdef DEBUG
    if (isnan(coeff)) {
      LOG(ERROR) << ComputeDist(entity_i, datum->entity_o(), datum->category_path());
      LOG(ERROR) << PLearn::fastsigmoid((-1.0) * ComputeDist(
        entity_i, datum->entity_o(), datum->category_path()));
    }
    CHECK(!isnan(coeff));
#endif
    AccumulateEntityGradient(coeff, datum->category_path()->aggr_dist_metric(), 
        entity_i, datum->entity_o(), entity_i_grad);
    // on e_o  
    // = (-1) * gradient_on_e_i, so simply do the copy
    datum->entity_o_grad()->CopyFrom(entity_i_grad, -1.0);

    // neg_samples
    for (int neg_idx = 0; neg_idx < num_neg_sample_; ++neg_idx) {
      Blob* neg_entity_grad = datum->neg_entity_grad(neg_idx);
      coeff = 1.0 - PLearn::fastsigmoid(ComputeDist(
          entity_i, datum->neg_entity(neg_idx), datum->category_path()));
#ifdef DEBUG
    CHECK(!isnan(coeff));
#endif
      //LOG(ERROR) << "neg_idx "  << neg_idx;
      AccumulateEntityGradient(
          (-1.0) * coeff, datum->neg_category_path(neg_idx)->aggr_dist_metric(), 
          entity_i, datum->neg_entity(neg_idx), neg_entity_grad);
      // Accumulate (-1) * gradient_on_neg_samples to gradient_on_e_i 
      //LOG(ERROR) << "start accu ";
      entity_i_grad->Accumulate(neg_entity_grad, -1.0);
      //LOG(ERROR) << "accu done";
    }
}

void Solver::ComputeCategoryGradient(Datum* datum) {
  // process (e_i,  e_o)
  const vector<int>& category_nodes = datum->category_path()->category_nodes();
  const int entity_i = datum->entity_i();
  const int entity_o = datum->entity_o();
  float coeff = PLearn::fastsigmoid((-1.0) * ComputeDist(
      entity_i, entity_o, datum->category_path())) - 1.0;
  for (int c_idx = 0; c_idx < category_nodes.size(); ++c_idx) {
    AccumulateCategoryGradient(coeff, entity_i, entity_o, 
        datum->category_grad(category_nodes[c_idx]));
  }

  // process (e_i, negative samples)
  const vector<Path*>& neg_category_paths = datum->neg_category_paths();
#ifdef DEBUG
  CHECK_EQ(neg_category_paths.size(), num_neg_sample_);
#endif
  for (int path_idx = 0; path_idx < neg_category_paths.size(); ++path_idx) {
    const vector<int>& neg_category_nodes 
        = neg_category_paths[path_idx]->category_nodes();
    const int neg_entity = datum->neg_entity(path_idx);
    float coeff = 1.0 - PLearn::fastsigmoid(ComputeDist(
        entity_i, neg_entity, neg_category_paths[path_idx]));
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

// Note: _single has different pipeline with _omp
//
void Solver::Solve_single(const vector<Datum*>& minibatch) {
#ifdef DEBUG
  CHECK(minibatch.size() != 0);
#endif
  float update_coeff = (-1.0) * learning_rate_ / minibatch.size();
  for (int epoch = 0; epoch < num_epoch_on_batch_; ++epoch) {
    // Refresh path aggregated distance metric
    for (int d = 0; d < minibatch.size(); ++d) {
      //LOG(ERROR) << "refreshing datum " << d;
      //CHECK(minibatch[d] != NULL);
      minibatch[d]->category_path()->RefreshAggrDistMetric(categories_);
      //LOG(ERROR) << "refreshing datum " << d << " done.";

      vector<Path*>& neg_category_paths = minibatch[d]->neg_category_paths();
      for (int p_idx = 0; p_idx < neg_category_paths.size(); ++p_idx) {
        //LOG(ERROR) << "refreshing datum " << d << " neg " << p_idx;
        neg_category_paths[p_idx]->RefreshAggrDistMetric(categories_);
      }
    }
    //LOG(ERROR) << "refresh path dist metric done."; 

    // Optimize entity embedding
    for (int iter = 0; iter < num_iter_on_entity_; ++iter) {
      for (int d = 0; d < minibatch.size(); ++d) {
        Datum* datum = minibatch[d];
        ComputeEntityGradient(datum);

        // Accumulate entity gradients
#ifdef DEBUG
        CHECK_GT(entity_grads_.size(), 0);
        //LOG(ERROR) << "[1] check 1 " << datum->entity_i();
        entity_grads_[datum->entity_i()]->CheckNaN();
        //LOG(ERROR) << "[1] check 2";
        datum->entity_i_grad()->CheckNaN();
#endif    
        //LOG(ERROR) << "[1] accu start";
        entity_grads_[datum->entity_i()]->Accumulate(
            datum->entity_i_grad(), 1.0);
        //LOG(ERROR) << "[1] accu done.";    
        entity_grads_[datum->entity_o()]->Accumulate(
            datum->entity_o_grad(), 1.0);
        //LOG(ERROR) << "[2] accu done.";
        if (epoch == 0 && iter == 0) {
          updated_entities_.insert(datum->entity_i());
          updated_entities_.insert(datum->entity_o());
        }
        for (int neg_idx = 0; neg_idx < num_neg_sample_; ++neg_idx) {
          //LOG(ERROR) << "[3] accu start.";
          entity_grads_[datum->neg_entity(neg_idx)]->Accumulate(
              datum->neg_entity_grad(neg_idx), 1.0);
          //LOG(ERROR) << "[3] accu done.";
          if (epoch == 0 && iter == 0) {
            updated_entities_.insert(datum->neg_entity(neg_idx));
          }
        }
      } // end of minibatch

      // Update entity vectors
      set_it_ = updated_entities_.begin();
      for (; set_it_ != updated_entities_.end(); ++set_it_) { 
        entities_[*set_it_]->Accumulate(entity_grads_[*set_it_], update_coeff);
        // Projection
        entities_[*set_it_]->Normalize();
        // clear gradidents
        entity_grads_[*set_it_]->ClearData();
      }
    } // end of optimizing entity embedding
    //LOG(ERROR) << "optimize entity vector done."; 

    // Optimize category embedding
    for (int iter = 0; iter < num_iter_on_category_; ++iter) {
      for (int d = 0; d < minibatch.size(); ++d) {
        Datum* datum = minibatch[d];
#ifdef DEBUG
          CHECK(datum);
#endif        

        if (iter > 0) {
          // Refresh path aggregated distance metric
#ifdef DEBUG
          CHECK(datum->category_path());
#endif      
          datum->category_path()->RefreshAggrDistMetric(categories_);
          vector<Path*>& neg_category_paths = datum->neg_category_paths();
          for (int p_idx = 0; p_idx < neg_category_paths.size(); ++p_idx) {
#ifdef DEBUG
            CHECK(neg_category_paths[p_idx]);
#endif        
            neg_category_paths[p_idx]->RefreshAggrDistMetric(categories_);
          }
        }

        ComputeCategoryGradient(datum); 

        // Accumulate category gradients
#ifdef DEBUG
        CHECK_GT(category_grads_.size(), 0);
#endif        
        const vector<Blob*>& datum_category_grads = datum->category_grads();
        map_it_ = datum->category_index().begin();
        for (; map_it_ != datum->category_index().end(); ++map_it_) {
          category_grads_[map_it_->first]->Accumulate(
              datum_category_grads[map_it_->second], 1.0);
          if (epoch == 0 && iter == 0) {
            updated_categories_.insert(map_it_->first);
          }
        }
      } // end of minibatch

      //Update category metrics
      set_it_ = updated_categories_.begin();
      for (; set_it_ != updated_categories_.end(); ++set_it_) {
#ifdef DEBUG
        //LOG(ERROR) << "cate " << *set_it_; 
        CHECK_LT(*set_it_, category_grads_.size());
        CHECK_LT(*set_it_, categories_.size());
#endif
        categories_[*set_it_]->Accumulate(category_grads_[*set_it_], update_coeff);
        // clear gradients
        category_grads_[*set_it_]->ClearData();
      }
    } // end of optimizing category embedding 
    //LOG(ERROR) << "optimize cate vector done."; 
  } // end of epoches
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
        vector<Path*>& neg_category_paths = datum->neg_category_paths();
        for (int p_idx = 0; p_idx < neg_category_paths.size(); ++p_idx) {
          neg_category_paths[p_idx]->RefreshAggrDistMetric(categories_);
        } 
      }
    }
  } // end of epoches
}

} // namespace entity
