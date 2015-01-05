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
#include <math.h>
#include <algorithm>

namespace entity {

  using namespace util;

//TODO
double expand_time = 0;
double find_time = 0;

  // Constructor
EEEngine::EEEngine() {
  entity::Context& context = entity::Context::get_instance();
  
  num_neg_sample_ = context.get_int32("num_neg_sample");
  
  num_train_data_ = 0;
  num_entity_ = 0;
  num_category_ = 0;
}

EEEngine::~EEEngine() { }

void EEEngine::ReadData() {
  string line;

  // Parse gflag
  entity::Context& context = entity::Context::get_instance();
  string dataset_path                 = context.get_string("dataset_path");
  string category_filename            = context.get_string("category_filename");
  string entity_filename              = context.get_string("entity_filename");
  string entity_to_ancestor_filename  = context.get_string("entity_to_ancestor_filename");
  string entity_to_category_filename  = context.get_string("entity_to_category_filename");
  string hierarchy_id_filename        = context.get_string("hierarchy_id_filename");
  string pair_filename                = context.get_string("pair_filename");
  string level_filename               = context.get_string("level_filename");
  
  // file streams
  ifstream category_file((dataset_path + "/" + category_filename).c_str());
  ifstream entity_file((dataset_path + "/" + entity_filename).c_str());
  ifstream hierarchy_id_file((dataset_path + "/" + hierarchy_id_filename).c_str());
  ifstream level_file((dataset_path + "/" + level_filename).c_str());

  if (!category_file.is_open())
    LOG(FATAL) << "fail to open:" << dataset_path + "/" + category_filename << "\n";
  if (!entity_file.is_open())
    LOG(FATAL) << "fail to open:" << dataset_path + "/" + entity_filename << "\n";
  if (!hierarchy_id_file.is_open())
    LOG(FATAL) << "fail to open:" << dataset_path + "/" + hierarchy_id_filename << "\n";
  if (!level_file.is_open())
    LOG(FATAL) << "fail to open:" << dataset_path + "/" + level_filename << "\n";

  // Parse Category File
  while (getline(category_file, line)){
    num_category_++;
  }
  LOG(INFO) << "number of category: " << num_category_;
 
  // Parse Entity File
  while (getline(entity_file, line)){
    num_entity_++;
  }
  LOG(INFO) << "number of entity: " << num_entity_;

  // Init category hierarchy
  entity_category_hierarchy_.InitHierarchy(num_entity_, num_category_);

  /// NOTE:
  // entity_id = index in nodes_;
  // category_id = index in nodes_ - num_entity_

  //
  ReadEntityCategoryFile(dataset_path + "/" + entity_to_category_filename);

  // parse hierarchy_id (category)
  LOG(INFO) << "Reading " << hierarchy_id_filename;
  while (getline(hierarchy_id_file, line)){
    istringstream iss(line);
    vector<int> tokens((istream_iterator<int>(iss)), istream_iterator<int>());
    
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
  // parse category levels 
  LOG(INFO) << "Reading " << level_filename;
  while (getline(level_file, line)){
    istringstream iss(line);
    vector<int> tokens((istream_iterator<int>(iss)), istream_iterator<int>());
    Node *category_node 
        = entity_category_hierarchy_.node(tokens[0] + num_entity_);
    category_node->set_level(tokens[1]);
  }

  ReadEntityAncestorFile_bin(dataset_path + "/" + entity_to_ancestor_filename);
  //ReadEntityAncestorFile_txt(dataset_path + "/" + entity_to_ancestor_filename);
  //ReadEntityAncestorFile_txt_bac(dataset_path + "/" + entity_to_ancestor_filename);

  ReadEntityPairFile(dataset_path + "/" + pair_filename);

  category_file.close();
  entity_file.close();
  hierarchy_id_file.close();
  level_file.close();

  LOG(INFO) << "Data reading done.";

}

void EEEngine::ReadEntityCategoryFile(const string& filename) {
  LOG(INFO) << "Reading " << filename;

  ifstream entity_category_file(filename.c_str());
  if (!entity_category_file.is_open()) {
    LOG(FATAL) << "fail to open:" << filename;
  }

  int counter = 0;
  string line;
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
    ++counter;
    if (counter % 200000 == 0) {
      cout << "." << std::flush;
    }
  }
  cout << endl;

  entity_category_file.close();
  CHECK_EQ(counter, num_entity_);
}

void EEEngine::ReadEntityPairFile(const string& filename) {
  entity_freq_.clear();
  entity_freq_.resize(num_entity_, 0); 

  ifstream pair_file(filename.c_str());
  if (!pair_file.is_open()) {
    LOG(FATAL) << "Fail to open " << filename;
  }

  LOG(INFO) << "Reading " << filename;
  num_train_data_ = 0;
  int entity_i, entity_o, count;
  while (pair_file >> entity_i) {
    pair_file >> entity_o >> count;
    if (entity_i == entity_o) {
      continue;
    }

    //TODO
    // Normalize to < 10
    //count = (count >  10 ? 10 : count);
    //for (int i = 0; i < count; ++i) {
      count = 1;
      train_data_.AddDatum(entity_i, entity_o, 1);

      num_train_data_++;
      entity_freq_[entity_i] += count;
      entity_freq_[entity_o] += count;
    //}
    
    if (num_train_data_ % 1000000 == 0) {
      cout << "." << flush;
    }
  }
  cout << endl;
  
  pair_file.close();
  LOG(INFO) << "number of training data: " << num_train_data_ << endl;
}

void EEEngine::ReadEntityAncestorFile_bin(const string& filename) {
  ifstream ancestor_file(filename.c_str(), ios::in | ios::binary);
  if (!ancestor_file.is_open()) {
    LOG(FATAL) << "Fail to open " << filename;
  }

  LOG(INFO) << "Reading " << filename;
  int counter = 0;
  int num_field, entity_id, ancestor_id;
  float rev_ancestor_weight;
  for (int line_id = 0; line_id < num_entity_; ++line_id) {
    ancestor_file.read((char*)&num_field, sizeof(int));
    ancestor_file.read((char*)&entity_id, sizeof(int));
    
    map<int, float>* ancestor_weight_map = new map<int, float>;
    for (int idx = 1; idx < num_field; ++idx) {
      ancestor_file.read((char*)&ancestor_id, sizeof(int));
      ancestor_file.read((char*)&rev_ancestor_weight, sizeof(float));
      (*ancestor_weight_map)[ancestor_id + num_entity_] 
          = (1.0 / rev_ancestor_weight);
    }
    entity_category_hierarchy_.AddAncestorWeights(
        entity_id, ancestor_weight_map);

    counter++;
    if (counter % 200000 == 0) {
      cout << "." << flush;
    }
  }
  cout << endl;
  ancestor_file.close();
}

void EEEngine::ReadEntityAncestorFile_txt(const string& filename) {
  ifstream ancestor_file(filename.c_str());
  if (!ancestor_file.is_open()) {
    LOG(FATAL) << "Fail to open " << filename;
  }

  LOG(INFO) << "Reading " << filename;
  int counter = 0;
  int num_field, entity_id, ancestor_id;
  float rev_ancestor_weight;
  while (ancestor_file >> num_field) {
    ancestor_file >> entity_id;

    map<int, float>* ancestor_weight_map = new map<int, float>;
    for (int idx = 1; idx < num_field; ++idx) {
      ancestor_file >> ancestor_id >> rev_ancestor_weight;

      (*ancestor_weight_map)[ancestor_id + num_entity_] 
          = (1.0 / rev_ancestor_weight);
    }
    entity_category_hierarchy_.AddAncestorWeights(
        entity_id, ancestor_weight_map);
    
    counter++;
    if (counter % 200000 == 0) {
      cout << "." << flush;
    }
  }
  cout << endl;
  ancestor_file.close();

#ifdef DEBUG
  CHECK_EQ(counter, num_entity_);
#endif
}

void EEEngine::ReadEntityAncestorFile_txt_bac(const string& filename) {
  //TODO
  ofstream bin_ancestor_file((filename + ".bin").c_str(), 
      ios::out | ios::binary);

  ifstream ancestor_file(filename.c_str());
  if (!ancestor_file.is_open()) {
    LOG(FATAL) << "Fail to open " << filename;
  }

  LOG(INFO) << "Reading " << filename;
  int counter = 0;
  string line;
  while (getline(ancestor_file, line)){
    istringstream iss(line);
    vector<string> tokens((istream_iterator<string>(iss)), 
        istream_iterator<string>());

    // entity_idx in heirarchy = entity_id
    const int entity_idx = stoi(tokens[0]);
    
    // TODO
    int size = tokens.size();
    int entity_id = entity_idx;
    bin_ancestor_file.write((char*)&size, sizeof(int));
    bin_ancestor_file.write((char*)&entity_id, sizeof(int));
  
    int ancestor_idx;
    float rev_ancestor_weight;
    //map<int, float>* ancestor_weight_map = new map<int, float>;
    for (int idx = 1; idx < tokens.size(); ++idx) {
      vector<string> ancestor_weight_pair = split(tokens[idx], ':');
      ancestor_idx = stoi(ancestor_weight_pair[0]) + num_entity_;
      rev_ancestor_weight = stof(ancestor_weight_pair[1]);
      //(*ancestor_weight_map)[ancestor_idx] = rev_ancestor_weight;
      
      //TODO
      int temp_ancestor_id = ancestor_idx - num_entity_;
      bin_ancestor_file.write((char*)&temp_ancestor_id, sizeof(int));
      bin_ancestor_file.write((char*)&rev_ancestor_weight, sizeof(float));
    }
    //entity_category_hierarchy_.AddAncestorWeights(
    //    entity_idx, ancestor_weight_map);

    ++counter;
    if (counter % 200000 == 0) {
      cout << "." << std::flush;
    }
  }
  cout << endl;
  ancestor_file.close();

  //TODO
  bin_ancestor_file.close();

#ifdef DEBUG
  CHECK_EQ(counter, num_entity_);
#endif
}

void EEEngine::BuildNoiseDistribution() {
  CHECK(entity_freq_.size() == num_entity_) << " ";

  entity_freq_[0] = pow(entity_freq_[0], 1.33333333);
  for (int e_id = 1; e_id < num_entity_; ++e_id) {
    entity_freq_[e_id] = pow(entity_freq_[e_id], 1.33333333);
    entity_freq_[e_id] += entity_freq_[e_id - 1];
  }
  freq_sum_ = entity_freq_[num_entity_ - 1];

#ifdef DEBUG
  LOG(INFO) << "BuildNoiseDistribution: sum = " << freq_sum_;
#endif
  LOG(INFO) << "Build Noise Distribution Done.";
}

void EEEngine::ThreadCreateMinibatch(const vector<int>* next_minibatch_data_idx,
    vector<Datum*>* next_minibatch) {
  const int batch_size = next_minibatch_data_idx->size();
#ifdef OPENMP
  #pragma omp parallel for
#endif
  for (int d_idx = 0; d_idx < batch_size; ++d_idx) {
    const int data_idx = next_minibatch_data_idx->at(d_idx);
    Datum* datum = train_data_.datum(data_idx);

    // compute path between entity_i and entity_o
    Path* entity_pair_path 
        = entity_category_hierarchy_.FindPathBetweenEntities(
        datum->entity_i(), datum->entity_o());
    datum->AddPath(entity_pair_path);

    // sample negative entities 
    SampleNegEntities(datum);
    
    (*next_minibatch)[d_idx] = datum;
  }
}

void EEEngine::SampleNegEntities(Datum* datum) {
  const int entity_i = datum->entity_i();
  const set<int>& pos_entities 
      = train_data_.positive_entities(entity_i);

  for (int neg_sample_idx = 0; neg_sample_idx < num_neg_sample_; 
      ++neg_sample_idx) { 
    int neg_entity = RandSampleNegEntity();
    while (neg_entity == entity_i || 
        pos_entities.find(neg_entity) != pos_entities.end() ||
        train_data_.positive_entities(neg_entity).find(entity_i) 
        != train_data_.positive_entities(neg_entity).end()) {
      neg_entity = RandSampleNegEntity();
    }
    // Generate path between entity_i and neg_sample
    Path* neg_path = entity_category_hierarchy_.FindPathBetweenEntities(
        entity_i, neg_entity);

    datum->AddNegSample(neg_sample_idx, neg_entity, neg_path);
  }
}

inline int EEEngine::RandSampleNegEntity() {
  double rn = ((double)rand() / RAND_MAX) * freq_sum_;
  return std::upper_bound(entity_freq_.begin(), entity_freq_.end(), rn) 
      - entity_freq_.begin();
  //return rand() % num_entity_;
}

inline void EEEngine::CopyMinibatch(const vector<Datum*>& source, 
    vector<Datum*>& target) {
  for (int idx = 0; idx < source.size(); ++idx) {
    target[idx] = source[idx];
  }
}

inline void EEEngine::ClearMinibatch(vector<Datum*>& minibatch) {
  // clear neg samples and related grads
  //for (int d_idx = 0; d_idx < minibatch.size(); ++d_idx){
  //  minibatch[d_idx]->ClearNegSamples();
  //}

  // destroy datum's
  for (int d_idx = 0; d_idx < minibatch.size(); ++d_idx){
    delete minibatch[d_idx];
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
  const string& output_file_prefix = context.get_string("output_file_prefix"); 
  const string& resume_path = context.get_string("resume_path");
  const int resume_iter = context.get_int32("resume_iter");

  int eval_counter = 0;

  BuildNoiseDistribution();

  // EEEL solver initialization
  Solver eeel_solver(num_entity_, num_category_);
  if (resume_path.length() > 0) {
    LOG(INFO) << "Resume from " << resume_path;
    CHECK_GE(resume_iter, 0);
    eeel_solver.Restore(resume_path, resume_iter);
  } else {
    eeel_solver.RandInit();
  }

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

  // skip initial minibatches
  for (int skip_idx = 0; skip_idx < resume_iter; ++skip_idx) {
    workload_mgr.IncreaseDataIdxByBatchSize();
  }
  // create the first minibatch
  workload_mgr.GetBatchDataIdx(workload_mgr.GetBatchSize(), 
      next_minibatch_data_idx);
  ThreadCreateMinibatch(&next_minibatch_data_idx, &next_minibatch);
  workload_mgr.IncreaseDataIdxByBatchSize();

  clock_t t_start = clock();
  double wait_time = 0;
  double test_time = 0;  

  // Train
  const int start_iter = (resume_iter > 1 ? resume_iter : 1); 	
  for (int iter = start_iter; iter <= num_iter; ++iter) {
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

    //LOG(INFO) << "SOLVE done.";
    
    ClearMinibatch(minibatch);

    if (iter % eval_interval == 0 || iter == start_iter) {
      clock_t t_start_test = clock();

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
          << ((double)(clock() - t_start) / CLOCKS_PER_SEC) << "," 
          << wait_time << "," << test_time; //<< "," << find_time << "," << expand_time;
      ++eval_counter;
      test = true;

      test_time += ((double)(clock() - t_start_test) / CLOCKS_PER_SEC);
      //LOG(INFO) << "test time," << test_time;
    }
    
    if (iter % snapshot == 0 && iter > start_iter) {
      eeel_solver.Snapshot(output_file_prefix, iter);
    }
    
    workload_mgr.IncreaseDataIdxByBatchSize();
    if (!test) {
      //LOG(INFO) << "wait " << wait_time;
      clock_t t_start_wait = clock();
      minibatch_creator.join();
      wait_time += ((double)(clock() - t_start_wait) / CLOCKS_PER_SEC);
    } else {
      test = false;
    }
  } // end of training
  
  if (num_iter % snapshot != 0) {
    eeel_solver.Snapshot(output_file_prefix, num_iter);
  }
}

}  // namespace entity
