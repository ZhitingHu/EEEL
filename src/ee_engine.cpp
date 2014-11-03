// Date: 2014.10.26
#include "gflags/gflags.h"
#include "glog/logging.h"

#include "ee_engine.hpp"
#include "solver.hpp"
#include "workload_manager.hpp"

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
  num_neg_sample_ = context.get_int32("num_neg_sample");
  num_entity_ = context.get_int32("num_entity");
}// constructor

// Destructor
EEEngine::~EEEngine() {
        //std::vector<Datum*>().swap(train_data_);
    train_data_.~Dataset();
        num_train_data_ = 0;
        num_test_data_ = 0;
}// destructor

void EEEngine::ReadData(const string& file_name) {

  // Read in training set.
  // format to be defined...
  
  //Dataset train_data_;
      
  for (int i = 0; i < 4000; ++i) {
    train_data_.Add_Datum(1, 2);
    //num_train_data_++;
  }
  /*
  num_train_data_++;
  for (int i = 0; i < num_data; ++i) {
    (*train_data_)[i] = new float[feature_dim];
    for (int j = 0; j < feature_dim; ++j) {
      infile >> (*train_data_)[i][j];
    }
  }
  */

}// readdata

void EEEngine::SampleNegEntities(const Datum* datum) {
  //TODO
}

void EEEngine::Start() {

  //petuum::PSTableGroup::RegisterThread();

  // Initialize local thread data structures.
  //int thread_id = thread_counter_++;
  int thread_id = 0;

  // Get flags
  entity::Context& context = entity::Context::get_instance();
  const int client_id = context.get_int32("client_id");
  const int num_client = context.get_int32("num_client");
  const int num_thread = context.get_int32("num_thread");
  const int num_epoch = context.get_int32("num_epoch");
  const int batch_size = context.get_int32("batch_size");
  const int num_batch_per_eval = context.get_int32("num_batch_per_eval");
  const int num_batch_per_epoch = context.get_int32("num_batch_per_epoch");
  const float learning_rate = context.get_double("learning_rate");

  int eval_counter = 0;
  int data_idx = 0;

  // EEPL solver initialization
  Solver eepl_solver(context.get_int32("num_entity"),
    context.get_int32("num_category"));

  if (client_id == 0 && thread_id == 0) {
    eepl_solver.RandInit();
  }

  // Workload Manager configuration
  WorkloadManagerConfig workload_mgr_config;
  workload_mgr_config.thread_id = thread_id;
  workload_mgr_config.client_id = client_id;
  workload_mgr_config.num_clients = num_client;
  workload_mgr_config.num_threads = num_thread;
  workload_mgr_config.num_batches_per_epoch = num_batch_per_epoch;
  workload_mgr_config.num_data = num_train_data_;
  WorkloadManager workload_mgr(workload_mgr_config);

  // Training	
  for (int epoch = 0; epoch < num_epoch; ++epoch) {
    int32_t num_batch_this_epoch = 0;
    workload_mgr.Restart();
    while (!workload_mgr.IsEnd()) {
      int32_t data_idx = workload_mgr.GetDataIdxAndAdvance();
      cout << data_idx << endl;

      // Create Minibatch 
      // Note: Consider remove minibatch and directly pass Dataset and an anchor
      vector<Datum*> minibatch;
      for (int minibatch_idx = 0; minibatch_idx < workload_mgr.GetBatchSize(); ++minibatch_idx){
        
        // add Datum to minibatch from Dataset 
        minibatch.push_back(train_data_.Get_Datum_adr(data_idx + minibatch_idx));

        // Negative Sampling
        for (int neg_sample_idx = 0; neg_sample_idx < num_neg_sample_; neg_sample_idx++){
          int32_t entity_id = minibatch[minibatch_idx]->entity_i();
          int32_t neg_entity_id = static_cast <int> (rand()) % static_cast <int> (num_entity_);
          const Path *neg_entity_path = train_data_.GetPath(entity_id, neg_entity_id);

          //TODO: consider handle duplicated case
          while (neg_entity_id == entity_id)
            neg_entity_id = static_cast <int> (rand()) % static_cast <int> (num_entity_);
          
          //cout << neg_entity_id << " ";
          minibatch[minibatch_idx]->AddNegSample(neg_entity_id, neg_entity_path);
        }
      }// minibatch
      
      //eepl_solver.Solve(minibatch);

      workload_mgr.IncreaseDataIdxByBatchSize();
      // end of batch handle
      if (workload_mgr.IsEndOfBatch()){
        ++num_batch_this_epoch;
        if (num_batch_this_epoch % num_batch_per_eval == 0) {
          //std::vector<int32_t> eval_data_idx = workload_mgr.GetBatchDataIdx(44);
          ++eval_counter;
          if (client_id == 0 && thread_id == 0) {
          }
        }
      }// end of batch
    } // is end workload manager
  } // epoch
  
}

}  // namespace entity
