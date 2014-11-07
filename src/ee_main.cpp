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


// Training Engine Parameters
DEFINE_int32(num_epoch, 1, "Number of data sweeps.");
DEFINE_int32(num_batch_per_eval, 1, "Number of batch per evaluation");
DEFINE_int32(num_batch_per_epoch, 50, "Number of batch per epoch");
DEFINE_int32(batch_size, 50, "Size of batch");

DEFINE_int32(client_id, 0, "Client ID");
DEFINE_int32(num_client, 1, "Number of client");
DEFINE_int32(num_thread, 1, "Number of thread");


// Solver Parameters
DEFINE_double(learning_rate, 0.1, "Initial step size");
DEFINE_int32(num_neg_sample, 50, "");
DEFINE_int32(dim_embedding, 100, "");
DEFINE_string(distance_metric_mode, "DIAG", "");
DEFINE_int32(num_epoch_on_batch, 1, "Number of data sweeps on a minibatch.");
DEFINE_int32(num_iter_on_entity, 1, "");
DEFINE_int32(num_iter_on_category, 1, "");
DEFINE_bool(openmp, false, "");

// Data
// to be derived from dataset, to merge to read_data
// I/O
DEFINE_string(dataset_path, "tech/", "data path");
DEFINE_string(output_file_prefix, "./", "Results go here.");
DEFINE_string(category_filename, "categories.txt", "category filename");
DEFINE_string(entity_filename, "entity.txt", "entity filename");
DEFINE_string(entity_to_ancestor_filename, "entity2ancestor.txt", "entity-ancestor filename");
DEFINE_string(entity_to_category_filename, "entity2category.txt", "entity-category filename");
DEFINE_string(hierarchy_filename, "hierarchy.txt", "hierarchy filename");
DEFINE_string(hierarchy_id_filename, "hierarchy_id.txt", "hierarchy id filename");
DEFINE_string(pair_filename, "pair.txt", "pair id filename");
DEFINE_string(level_filename, "level.txt", "category level filename");

//DEFINE_int32(num_entity, 1000, "");
//DEFINE_int32(num_train_data, 4000, "Number of training data.");
DEFINE_int32(num_test_data, 0, "Number of testing data.");
//DEFINE_int32(num_category, 1000, "");  



int main(int argc, char *argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  entity::EEEngine ee_engine;


  // read data
  ee_engine.ReadData("data.txt");
  // training
  //ee_engine.Start();

  LOG(INFO) << "Entity Embedding finished and shut down!";
  cout << "Process done" << endl; 
  system("pause");
 
  return 0;
}

