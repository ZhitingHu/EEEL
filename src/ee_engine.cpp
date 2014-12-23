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
#include <iterator>
#include <vector>
#include <stdint.h>
#include <thread>
#include <time.h>
#include <omp.h>

namespace entity {

  using namespace util;

  // Constructor
EEEngine::EEEngine() {
  entity::Context& context = entity::Context::get_instance();
  
  num_neg_sample_ = context.get_int32("num_neg_sample");
  
  num_train_data_ = 0;
  num_entity_ = 0;
  num_category_ = 0;
}

EEEngine::~EEEngine() { }

// Read and parse data
void EEEngine::ReadData() {
  string line;
  int counter;

  // Parse gflag
  entity::Context& context = entity::Context::get_instance();
  string dataset_path                 = context.get_string("dataset_path");
  string category_filename            = context.get_string("category_filename");
  string entity_filename              = context.get_string("entity_filename");
  string entity_to_ancestor_filename  = context.get_string("entity_to_ancestor_filename");
  string entity_to_category_filename  = context.get_string("entity_to_category_filename");
  string hierarchy_filename           = context.get_string("hierarchy_filename");
  string hierarchy_id_filename        = context.get_string("hierarchy_id_filename");
  string pair_filename                = context.get_string("pair_filename");
  string level_filename               = context.get_string("level_filename");
  
  // file streams
  ifstream category_file((dataset_path + "/" + category_filename).c_str());
  ifstream entity_file((dataset_path + "/" + entity_filename).c_str());
  ifstream entity_ancestor_file((dataset_path + "/" + entity_to_ancestor_filename).c_str());
  ifstream entity_category_file((dataset_path + "/" + entity_to_category_filename).c_str());
  ifstream hierarchy_file((dataset_path + "/" + hierarchy_filename).c_str());
  ifstream hierarchy_id_file((dataset_path + "/" + hierarchy_id_filename).c_str());
  ifstream pair_file((dataset_path + "/" + pair_filename).c_str());
  ifstream level_file((dataset_path + "/" + level_filename).c_str());

  if (!category_file.is_open())
    cout << "fail to open:" << dataset_path + category_filename << "\n";
  if (!entity_file.is_open())
    cout << "fail to open:" << dataset_path + entity_filename << "\n";
  if (!entity_ancestor_file.is_open())
    cout << "fail to open:" << dataset_path + entity_to_ancestor_filename << "\n";
  if (!entity_category_file.is_open())
    cout << "fail to open:" << dataset_path + entity_to_category_filename << "\n";
  if (!hierarchy_file.is_open())
    cout << "fail to open:" << dataset_path + hierarchy_filename << "\n";
  if (!hierarchy_id_file.is_open())
    cout << "fail to open:" << dataset_path + hierarchy_id_filename << "\n";
  if (!pair_file.is_open())
    cout << "fail to open:" << dataset_path + pair_filename << "\n";
  if (!level_file.is_open())
    cout << "fail to open:" << dataset_path + level_filename << "\n";

  // TODO: no need of category_file @hzt
  // 1. Parse Category File
  while (getline(category_file, line)){
    num_category_++;
  }
  cout << "number of category: " << num_category_ << endl;
  LOG(INFO) << "number of category: " << num_category_;
 
  // 2. Parse Entity File
  while (getline(entity_file, line)){
    num_entity_++;
  }
  cout << "number of entity: " << num_entity_ << endl;

  // init category hierarchy
  entity_category_hierarchy_.InitHierarchy(num_entity_, num_category_);

  // entity_id = index in nodes_;
  // category_id = index in nodes_ - num_entity

  // 3. Parse entity file
  while (getline(entity_category_file, line)) {
    istringstream iss(line);
    vector<int> tokens( 
        (std::istream_iterator<int>(iss)), std::istream_iterator<int>());
    
    // entity_idx in hierarchy = entity_id
    const int entity_idx = tokens[0];
    Node* entity_node = entity_category_hierarchy_.node(entity_idx);
    for (int p_idx = 1; p_idx < tokens.size(); ++p_idx) {
      const int category_id = tokens[p_idx];
      const int category_idx = category_id + num_entity_;
      // add parent categories to entity
      entity_node->AddParent(category_idx);
      // add child entity to category @hzt
      entity_category_hierarchy_.node(category_idx)->AddChild(entity_idx);
    }
  }

  // 4. Parse hierarchy_id (category)
  while (getline(hierarchy_id_file, line)){
    istringstream iss(line);
    vector<int> tokens((istream_iterator<int>(iss)), istream_iterator<int>());
    
    // revised by @hzt
    const int category_idx = tokens[0] + num_entity_;
    Node *category_node 
        = entity_category_hierarchy_.node(category_idx);
    for (int idx = 1; idx < tokens.size(); ++idx){
      const int child_category_idx = tokens[idx] + num_entity_;
      // add child categories to category
      category_node->AddChild(child_category_idx);
      // add parent categories to current category
      entity_category_hierarchy_.node(child_category_idx)->
          AddParent(category_idx);
    }
  }
 
  // category levels 
  while (getline(level_file, line)){
    istringstream iss(line);
    vector<int> tokens((istream_iterator<int>(iss)), istream_iterator<int>());
    Node *category_node 
        = entity_category_hierarchy_.node(tokens[0] + num_entity_);
    category_node->set_level(tokens[1]);
  }

  // 5. Parse entity ancestor File
  int num_entity_ancetor_file_line = 0;
  while (getline(entity_ancestor_file, line)){
    istringstream iss(line);
    vector<string> tokens((istream_iterator<string>(iss)), 
        istream_iterator<string>());
    //743:8.3
    // entity_idx in heirarchy= entity_id
    const int entity_idx = stoi(tokens[0]);
    Node *entity_node = entity_category_hierarchy_.node(entity_idx);
    
    // revised by @hzt
    map<int, float>* ancestor_weight_map = new map<int, float>;
    for (int idx = 1; idx < tokens.size(); ++idx) {
      vector<string> ancestor_weight_pair = split(tokens[idx], ':');
      const int ancestor_idx = stoi(ancestor_weight_pair[0]) + num_entity_;
      (*ancestor_weight_map)[ancestor_idx] = stof(ancestor_weight_pair[1]);
      entity_category_hierarchy_.AddAncestorWeights(
          entity_idx, ancestor_weight_map);
    }

    ++num_entity_ancetor_file_line;
  }
#ifdef DEBUG
  CHECK_EQ(num_entity_ancetor_file_line, num_entity_);
#endif
  
  // 6. Parse pair File
  // Remark: current memory allocation is not good, 
  // consider allocate once than assign in future version.
  while (getline(pair_file, line)) {
    istringstream iss(line);
    //vector<int> tokens{istream_iterator<int>{iss},istream_iterator<int>{}};
    vector<int> tokens((istream_iterator<int>(iss)), istream_iterator<int>());

    // revized by @hzt
    const int entity_i = tokens[0];
    const int entity_o = tokens[1];
    const int count = tokens[2];

    //Path* entity_pair_path 
    //    = entity_category_hierarchy_.FindPathBetweenEntities(
    //    entity_i, entity_o);
    //train_data_.AddDatum(entity_i, entity_o, count, entity_pair_path);
    train_data_.AddDatum(entity_i, entity_o, count);
    
    num_train_data_++;
  }
#ifdef OPENMP
  #pragma omp parallel for
#endif
  for (int d_idx = 0; d_idx < num_train_data_; ++d_idx) {
    Datum* datum = train_data_.datum(d_idx);
    Path* entity_pair_path 
        = entity_category_hierarchy_.FindPathBetweenEntities(
        datum->entity_i(), datum->entity_o());
    datum->AddPath(entity_pair_path);
#ifdef OPENMP
  #pragma omp critical
#endif
    train_data_.AddPath(datum->entity_i(), datum->entity_o(), entity_pair_path);
  }
  cout << "number of training data: " << num_train_data_ << endl;

  // close files
  category_file.close();
  entity_file.close();
  entity_ancestor_file.close();
  entity_category_file.close();
  hierarchy_file.close();
  hierarchy_id_file.close();
  pair_file.close();
  level_file.close();
}

void EEEngine::ThreadCreateMinibatch(const vector<int>* next_minibatch_data_idx,
    vector<Datum*>* next_minibatch) {
  //LOG(INFO) << "=== start";
  const int batch_size = next_minibatch_data_idx->size();
#ifdef OPENMP
  #pragma omp parallel for
#endif
  for (int d_idx = 0; d_idx < batch_size; ++d_idx) {
    const int data_idx = next_minibatch_data_idx->at(d_idx);
    Datum* datum = train_data_.datum(data_idx);
    SampleNegEntities(datum);
    
    (*next_minibatch)[d_idx] = datum;
  }
  //LOG(INFO) << "=== end";
}

void EEEngine::SampleNegEntities(Datum* datum) {
  const int entity_i = datum->entity_i();
  const map<int, Path*>& pos_entities 
      = train_data_.positive_entity_path(entity_i);

  for (int neg_sample_idx = 0; neg_sample_idx < num_neg_sample_; 
      ++neg_sample_idx) { 
    // TODO use sophisiticated distribution
    int neg_entity = rand() % num_entity_;
    while (neg_entity == entity_i || 
        pos_entities.find(neg_entity) != pos_entities.end()) {
      neg_entity = rand() % num_entity_;
    }
    // Generate path between entity_i and neg_sample
    Path* neg_path = entity_category_hierarchy_.FindPathBetweenEntities(
        entity_i, neg_entity);

    datum->AddNegSample(neg_sample_idx, neg_entity, neg_path);
  }
}

inline void EEEngine::CopyMinibatch(const vector<Datum*>& source, 
    vector<Datum*>& target) {
  for (int idx = 0; idx < source.size(); ++idx) {
    target[idx] = source[idx];
  }
}

inline void EEEngine::ClearMinibatch(vector<Datum*>& minibatch) {
  // clear neg samples and related grads
  for (int d_idx = 0; d_idx < minibatch.size(); ++d_idx){
    minibatch[d_idx]->ClearNegSamples();
  }
}

void EEEngine::Start() {
  int thread_id = 0;

  entity::Context& context = entity::Context::get_instance();
  const int client_id = context.get_int32("client_id");
  const int num_client = context.get_int32("num_client");
  const int num_thread = context.get_int32("num_thread");
  const int num_iter = context.get_int32("num_iter");
  const int batch_size = context.get_int32("batch_size");
  const int eval_interval = context.get_int32("eval_interval");
  const int num_iter_per_eval = context.get_int32("num_iter_per_eval");
  const int snapshot = context.get_int32("snapshot");
  const float learning_rate = context.get_double("learning_rate");
  const string& output_file_prefix = context.get_string("output_file_prefix"); 

  int eval_counter = 0;
  int data_idx = 0;

  // EEEL solver initialization
  Solver eeel_solver(num_entity_, num_category_);
  eeel_solver.RandInit();

  // workload manager configuration
  WorkloadManagerConfig workload_mgr_config;
  workload_mgr_config.thread_id = thread_id;
  workload_mgr_config.client_id = client_id;
  workload_mgr_config.num_clients = num_client;
  workload_mgr_config.num_threads = num_thread;
  workload_mgr_config.batch_size = batch_size;
  workload_mgr_config.num_data = num_train_data_;
  WorkloadManager workload_mgr(workload_mgr_config);

  // pre-computed minibatch
  vector<int> next_minibatch_data_idx(workload_mgr.GetBatchSize());
  vector<Datum*> next_minibatch(workload_mgr.GetBatchSize());
  // current-used minibatch
  vector<Datum*> minibatch(workload_mgr.GetBatchSize());
  // pre-computed test minibatch
  vector<int> next_test_minibatch_data_idx(workload_mgr.GetBatchSize());
  vector<Datum*> next_test_minibatch(workload_mgr.GetBatchSize());
  // current-used test minibatch
  vector<Datum*> test_minibatch(workload_mgr.GetBatchSize());
  bool test = false;
  thread minibatch_creator;

  // create the first minibatch 
  for (int d_idx = 0; d_idx < workload_mgr.GetBatchSize(); ++d_idx) {
    int data_idx = workload_mgr.GetDataIdxAndAdvance();
    Datum* datum = train_data_.datum(data_idx);
    SampleNegEntities(datum);

    next_minibatch[d_idx] = datum;
  }
  
  clock_t t_start = clock();

  // Train	
  for (int iter = 1; iter <= num_iter; ++iter) {
    // get current minibatch from pre-computed one
    CopyMinibatch(next_minibatch, minibatch);
    // pre-compute next minibatch
    workload_mgr.GetBatchDataIdx(workload_mgr.GetBatchSize(), 
        next_minibatch_data_idx);
    //LOG(INFO) << "start thread";
    minibatch_creator = thread(&EEEngine::ThreadCreateMinibatch, this, 
        &next_minibatch_data_idx, &next_minibatch);
    //ThreadCreateMinibatch(&next_minibatch_data_idx, &next_minibatch);
    //LOG(INFO) << "end thread";

    // TODO
    //LOG(INFO) << workload_mgr.GetDataIdx() << " " 
    //    << ((double)(clock() - t_start) / CLOCKS_PER_SEC);

    // optimize based on current minibatch
    eeel_solver.Solve(minibatch);
    
    ClearMinibatch(minibatch);

    if (iter % eval_interval == 0 || iter == 1) {
      float obj = 0;
      int cur_batch_start_idx = workload_mgr.GetDataIdx();
      // get the first test minibatch
      minibatch_creator.join();
      CopyMinibatch(next_minibatch, test_minibatch);
      for (int test_iter = 0; test_iter < num_iter_per_eval; ++test_iter) {

        // TODO
        //LOG(INFO) << "test " << cur_batch_start_idx << " "
        //    << ((double)(clock() - t_start) / CLOCKS_PER_SEC);

        if (test_iter < num_iter_per_eval - 1) {
          // pre-compute next test minibatch
          cur_batch_start_idx 
              = workload_mgr.GetNextBatchStartIdx(cur_batch_start_idx);
          workload_mgr.GetBatchDataIdx(workload_mgr.GetBatchSize(), 
              next_test_minibatch_data_idx, cur_batch_start_idx);
          minibatch_creator = thread(&EEEngine::ThreadCreateMinibatch, this,
              &next_test_minibatch_data_idx, &next_test_minibatch);
          //ThreadCreateMinibatch(&next_test_minibatch_data_idx, &next_test_minibatch);
        }

        // use current minibatch for test
        obj += eeel_solver.ComputeObjective(test_minibatch);

        // preserve the first test minibatch for training usage
        if (test_iter > 0) { 
          ClearMinibatch(test_minibatch);
        }

        if (test_iter < num_iter_per_eval - 1) {    
          minibatch_creator.join();
          CopyMinibatch(next_test_minibatch, test_minibatch);
        }
      }
      
      obj /= (workload_mgr.GetBatchSize() * num_iter_per_eval); 
      LOG(ERROR) << iter << "," << obj << "," 
          << ((double)(clock() - t_start) / CLOCKS_PER_SEC);
      ++eval_counter;
      test = true;
    }
    
    if (iter % snapshot == 0) {
      eeel_solver.Snapshot(output_file_prefix, iter);
    }
    
    workload_mgr.IncreaseDataIdxByBatchSize();
    if (!test) {
      //LOG(INFO) << "wait";
      minibatch_creator.join();
    } else {
      test = false;
    }
  } // end of training
  
  if (num_iter % snapshot != 0) {
    eeel_solver.Snapshot(output_file_prefix, num_iter);
  }
}

}  // namespace entity
