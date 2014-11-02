// Date: 2014.10.26
#include "gflags/gflags.h"
#include "glog/logging.h"

#include "ee_engine.hpp"
#include "solver.hpp"

#include "util.hpp"
#include "string_util.hpp"
#include <string>
#include <fstream>
#include <vector>
#include <cstdint>

namespace entity {

namespace {
  // Use the next kNumDataEval data per thread to evaluate objective.
  const int kNumDataEval = 100;
}  // anonymous namespace

  // Constructor
EEEngine::EEEngine(){
  entity::Context& context = entity::Context::get_instance();
  //std::string meta_file = context.get_string("dataset_path") + ".meta";
  //std::ifstream is(meta_file.c_str());

  // parameter initialization
  num_train_data_ = context.get_int32("num_train_data");
  num_test_data_ = context.get_int32("num_test_data");
}// constructor

// Destructor
EEEngine::~EEEngine() {
        //std::vector<Datum*>().swap(train_data_);
    train_data_.~Dataset();
        num_train_data_ = 0;
        num_test_data_ = 0;
}// destructor

void EEEngine::ReadData() {
  // Read in training set.
  // format to be defined...
  
  //Dataset train_data_;
      
        /*
        for (int i = 0; i < num_data; ++i) {
      	  (*train_data_)[i] = new float[feature_dim];
      	  for (int j = 0; j < feature_dim; ++j) {
      		  infile >> (*train_data_)[i][j];
      	  }
        }*/
}// readdata

void EEEngine::SampleNegEntities(const Datum* datum) {
  //TODO
}

void EEEngine::Start() {
  entity::Context& context = entity::Context::get_instance();
  const int num_epoch = context.get_int32("num_epoch");
  const int num_batch_per_eval = context.get_int32("num_batch_per_eval");
  int eval_counter = 0;
  int data_idx = 0;

  Solver eepl_solver(context.get_int32("num_entity"),
      context.get_int32("num_category"));

  eepl_solver.RandInit();

  // Training	
  for (int epoch = 0; epoch < num_epoch; ++epoch) {
    int num_batches_this_epoch = 0;
    for (int batch = 0; batch < num_batch_per_eval; ++batch){
      // TODO: Create Minibatch  (and negative sampling)
      
      //eepl_solver.Solve();

      if (1 /*workload_mgr.IsEndOfBatch()*/) { 
        ++num_batches_this_epoch;
        /*
        if (num_batches_this_epoch % num_batches_per_eval == 0) {
      
                ++eval_counter;
        }*/
        }                   
     }
  }
}

}  // namespace entity
