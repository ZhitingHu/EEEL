// Author: Zhi-Ting Hu, Po-Yao Huang
// Date: 2014.10.26
#ifndef ENTITY_SOLVER_HPP_
#define ENTITY_SOLVER_HPP_

#include "blob.hpp"
#include "datum.hpp"
#include <stdint.h>
#include <vector>
#include <set>

namespace entity {

class Solver {
public:
  Solver(const int num_entity, const int num_category);
  Solver(): num_entity_(-1), num_category_(-1), dim_embedding_(-1) {}

  ~Solver();
  
  // Initialization
  //void Init(); => merge into constructore
  void RandInit();

  // Optimizes based on minibatch
  // Assumes symmetric distance, i.e. d(e_i, e_o) = d(e_o, e_i)
  void Solve(const vector<Datum*>& minibatch);
  
  void Solve_single(const vector<Datum*>& minibatch);
  void Solve_omp(const vector<Datum*>& minibatch);

  void Snapshot(const string& output_path, const int iter);
  void Restore(const string& snapshot_path, const int iter);

  const float ComputeObjective(const vector<Datum*>& val_batch);
  const float ComputeObjective_single(const vector<Datum*>& val_batch);
  const float ComputeObjective_omp(const vector<Datum*>& val_batch);
  
  const float ComputeDist(const int entity_from, const int entity_to, 
      const Path* path);

  const int num_entity() { return num_entity_; }
  const int dim_embedding() { return dim_embedding_; }
  const int num_category() { return num_category_; }
  const vector<Blob*> entities() { return entities_; }
  const vector<Blob*> categories() { return categories_; }

private:
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

  void SGDUpdateEntity(const float lr);
  void SGDUpdateCategory(const float lr);
  void MomenUpdateEntity(const float lr);
  void MomenUpdateCategory(const float lr);
  void AdaGradUpdateEntity(const float lr);
  void AdaGradUpdateCategory(const float lr);

  void SnapshotParameters(const string& param_filename);
  void SnapshotBlobs(const string& blobs_filename, const vector<Blob*>& blobs);
  void SnapshotBlobsBinary(const string& blobs_filename, const vector<Blob*>& blobs);

  void RestoreParameters(const string& param_filename);
  void RestoreBlobs(const string& blobs_filename, vector<Blob*>& blobs);
  void RestoreBlobsBinary(const string& blobs_filename, const int num_blob, 
      vector<Blob*>& blobs);

private:
  enum SolverType {
    SGD = 0,
    MOMEN,
    ADAGRAD
  };
  SolverType solver_type_;
  bool restore_history_;

  vector<Blob*> entities_;
  vector<Blob*> categories_;
  
  // Used in single-thread version
  vector<Blob*> entity_grads_;
  vector<Blob*> category_grads_;
  set<int> updated_entities_;
  set<int> updated_categories_;
  set<int>::const_iterator set_it_;
  map<int, int>::const_iterator map_it_;
 
  // Used in Momentum/AdaGrad solver types
  float momentum_;
  vector<Blob*> entity_update_history_;
  vector<Blob*> category_update_history_;
  
  // 
  int num_entity_;
  int num_category_;

  int num_neg_sample_;
  int dim_embedding_;
  float learning_rate_; //TODO adaptive lr
  int num_epoch_on_batch_;
  int num_iter_on_entity_;
  int num_iter_on_category_;
  int snapshot_;
};

}  // namespace entity

#endif
