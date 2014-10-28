// Author: Zhi-Ting Hu, Po-Yao Huang
// Date: 2014.10.26
#include "gflags/gflags.h"	// For VC development only
#include "glog/logging.h"	// For VC development only
#include <thread>
#include <vector>
#include <cstdint>
#include <iostream>
#include <numeric>	// test use only
#include "context.hpp"
#include "ee_engine.hpp"


using namespace std;

// entity embedding parameters
// I/O
DEFINE_string(dataset_path, "", "data path");
// Data
DEFINE_int32(num_train_data, 1000, "Number of training data.");
DEFINE_int32(num_test_data, 0, "Number of testing data.");
// Traiing Engine Parameters
DEFINE_int32(num_epochs, 1, "Number of data sweeps.");
DEFINE_int32(num_batches_per_eval, 10, "Number of batches per evaluation");
DEFINE_int32(batch_size, 50, "");
// Learning/Solver Parameters
DEFINE_double(learning_rate, 0.1, "Initial step size");
DEFINE_int32(num_neg_sample, 50, "");
DEFINE_int32(dim_entity_vector, 100, "");
DEFINE_int32(distance_metric_mode, 0, "");

DEFINE_int32(num_category, 1000, "");  // to be derived from dataset
DEFINE_int32(num_entity, 1000, "");     // tp be derived form dataset

// Misc
DEFINE_string(output_file_prefix, "", "Results go here.");


int main(int argc, char *argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  entity::EEEngine ee_engine;


  // read data
  ee_engine.ReadData();
  // training
  ee_engine.Start();

  LOG(INFO) << "Entity Embedding finished and shut down!";
  cout << "Process done" << endl;
  system("pause");
  return 0;
}

