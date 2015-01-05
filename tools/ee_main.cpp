// Author: Zhi-Ting Hu, Po-Yao Huang
// Date: 2014.10.26
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "context.hpp"
#include "ee_engine.hpp"

#include <thread>
#include <vector>
#include <cstdint>
#include <iostream>
#include <numeric>	// test use only


using namespace std;

// entity embedding parameters
DEFINE_int32(dim_embedding, 100, "");
DEFINE_string(distance_metric_mode, "DIAG", "");

// Training Engine Parameters
DEFINE_int32(num_iter, 1, "Number of iteration.");
DEFINE_int32(eval_interval, 1, "Number of iteration between two evaluations");
DEFINE_int32(num_iter_per_eval, 1, "Number of iteration per evaluation");
DEFINE_int32(batch_size, 50, "Size of batch");
DEFINE_int32(snapshot, 50, "Number of iterations between two snapshots");
DEFINE_string(resume_path, "", "Results to be resumed");
DEFINE_int32(resume_iter, -1, "Iteration of results");

// Parameter Server (not used)
DEFINE_int32(client_id, 0, "Client ID");
DEFINE_int32(num_client, 1, "Number of client");
DEFINE_int32(num_thread, 1, "Number of thread");

// Solver Parameters
DEFINE_double(learning_rate, 0.1, "Initial step size");
DEFINE_int32(num_neg_sample, 50, "");
DEFINE_int32(num_epoch_on_batch, 1, "Number of data sweeps on a minibatch.");
DEFINE_int32(num_iter_on_entity, 1, "");
DEFINE_int32(num_iter_on_category, 1, "");
DEFINE_bool(openmp, false, "");

// Data
// to be derived from dataset, to merge to read_data
// I/O
DEFINE_string(dataset_path, "data/tech/", "data path");
DEFINE_string(output_file_prefix, "output/", "Results go here.");
DEFINE_string(category_filename, "categories.txt", "category filename");
DEFINE_string(entity_filename, "entity.txt", "entity filename");
DEFINE_string(entity_to_ancestor_filename, "entity2ancestor.bin", "entity-ancestor filename");
DEFINE_string(entity_to_category_filename, "entity2category.txt", "entity-category filename");
DEFINE_string(hierarchy_id_filename, "hierarchy_id.txt", "hierarchy id filename");
DEFINE_string(pair_filename, "pair.txt", "pair id filename");
DEFINE_string(level_filename, "level.txt", "category level filename");

DEFINE_int32(num_test_data, 0, "Number of testing data.");

int main(int argc, char *argv[]) {
  FLAGS_alsologtostderr = 1;
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  entity::EEEngine ee_engine;

  // read data
  ee_engine.ReadData();
  // training
  //ee_engine.Start();

  LOG(INFO) << "Entity Embedding finished and shut down!";
 
  return 0;
}

