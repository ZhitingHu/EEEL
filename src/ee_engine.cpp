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
#include <cstdint>

namespace entity {

  using namespace util;

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
  
  num_test_data_ = context.get_int32("num_test_data");
  num_neg_sample_ = context.get_int32("num_neg_sample");
  
  num_train_data_ = 0; //context.get_int32("num_train_data");
  num_entity_ = 0; //context.get_int32("num_entity");
  num_category_ = 0;
}// constructor

// Destructor
EEEngine::~EEEngine() {
        //std::vector<Datum*>().swap(train_data_);
    train_data_.~Dataset();
    num_train_data_ = 0;
    num_test_data_ = 0;
}// destructor

// Read and parse data
void EEEngine::ReadData(const string& file_name) {
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
  ifstream category_file(dataset_path + "/" + category_filename);
  ifstream entity_file(dataset_path + "/" + entity_filename);
  ifstream entity_ancestor_file(dataset_path + "/" + entity_to_ancestor_filename);
  ifstream entity_category_file(dataset_path + "/" + entity_to_category_filename);
  ifstream hierarchy_file(dataset_path + "/" + hierarchy_filename);
  ifstream hierarchy_id_file(dataset_path + "/" + hierarchy_id_filename);
  ifstream pair_file(dataset_path + "/" + pair_filename);
  ifstream level_file(dataset_path + "/" + level_filename);

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
  while (getline(entity_category_file, line)){
    istringstream iss(line);
    vector<int> tokens{ 
        std::istream_iterator<int>{iss}, std::istream_iterator<int>{}};
    
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
    vector<int> tokens{istream_iterator<int>{iss}, istream_iterator<int>{}};
    
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
    //for (int level = 0; level < tokens.size(); ++level){
    //  Node *category_node 
    //      = entity_category_hierarchy_.node(tokens[level] + num_entity_);

    //  // set node/level
    //  //temp_category_node->set_Node(tokens[level], level);

    //  // add child categories to current category
    //  if (level + 1 < tokens.size()) {
    //    category_node->AddChild(tokens[level + 1] + num_entity_);
    //  }

    //  // add parent categories to current category
    //  if (level > 0) {
    //    category_node->AddParent(tokens[level - 1] + num_entity_);
    //  }
    //}
  }
 
  // category levels 
  while (getline(level_file, line)){
    istringstream iss(line);
    vector<int> tokens{ istream_iterator<int>{iss}, istream_iterator<int>{} };
    Node *category_node 
        = entity_category_hierarchy_.node(tokens[0] + num_entity_);
    category_node->set_level(tokens[1]);
  }

  // 5. Parse entity ancestor File
  int num_entity_ancetor_file_line = 0;
  while (getline(entity_ancestor_file, line)){
    istringstream iss(line);
    vector<string> tokens{ istream_iterator<string>{iss}, istream_iterator<string>{} };
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

    //map<int, float>* ancestor_weight_map;
    //// add Entity Ancestor 
    //for (int level = 1; level < tokens.size(); ++level){
    //  vector<string> temp_string = split(tokens[level], ':');
    //  ancestor_weight_map = new map<int, float>;
    //  ancestor_weight_map->insert(pair<int, float>(stoi(temp_string[0]), stof(temp_string[1])));
    //  
    //  // this step is extremely slow, it takes ~5min to store these maps 
    //  // modify to vector<*map> instead of vector<map>
    //  entity_category_hierarchy_.Add_weights(temp_map);
    //}
  }
#ifdef DEBUG
  CHECK_EQ(num_entity_ancetor_file_line, num_entity_);
#endif
  
  // 6. Parse pair File
  // Remark: current memory allocation is not good, 
  // consider allocate once than assign in future version.
  while (getline(pair_file, line)){
    istringstream iss(line);
    vector<int> tokens{ istream_iterator<int>{iss},istream_iterator<int>{}};

    // revized by @hzt
    const int entity_i = tokens[0];
    const int entity_o = tokens[1];
    const int count = tokens[2];
    train_data_.AddDatum(entity_i, entity_o, count);

    // Add path to pairs
    Path* entity_pair_path 
        = entity_category_hierarchy_.FindPathBetweenEntities(
        entity_i, entity_o);
    train_data_.AddPath(entity_i, entity_o, entity_pair_path);
    
    num_train_data_++;
    //Datum *temp = train_data_.datum(num_train_data_ - 1);
    //cout << temp->entity_i() << "\t" << temp->entity_o() << "\t" << temp->count() << endl;
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

void EEEngine::SampleNegEntities(const Datum* datum) {
  const int entity_i = datum->entity_i();
  const map<int, Path*>& pos_entities 
      = train_data_.entity_pair_path()[entity_i];

  for (int neg_sample_idx = 0; neg_sample_idx < num_neg_sample_; 
      ++neg_sample_idx) 
    // TODO use sophisiticated distribution
    int neg_entity = rand() % num_entity_;
    while (neg_entity == entity_i || 
        pos_entities.find(neg_entity) != pos_entities.end()) {
      neg_entity_id = rand() % num_entity_;
    }
    // Generate path between entity_i and neg_sample
    Path* neg_path = entity_category_hierarchy_.FindPathBetweenEntities(
        entity_i, neg_entity);

    datum->AddNegSample(neg_entity, neg_path);
  }
}

void EEEngine::Start() {
  int thread_id = 0;

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
  Solver eeel_solver(num_entity_, num_category_);

  if (client_id == 0 && thread_id == 0) {
    eeel_solver.RandInit();
  }

  // Workload Manager configuration
  WorkloadManagerConfig workload_mgr_config;
  workload_mgr_config.thread_id = thread_id;
  workload_mgr_config.client_id = client_id;
  workload_mgr_config.num_clients = num_client;
  workload_mgr_config.num_threads = num_thread;
  //workload_mgr_config.num_batches_per_epoch = num_batch_per_epoch;
  workload_mgr_config.batch_size = batch_size;
  workload_mgr_config.num_data = num_train_data_;
  WorkloadManager workload_mgr(workload_mgr_config);

  // Training	
  //for (int epoch = 0; epoch < num_epoch; ++epoch) {
  vector<Datum*> minibatch(workload_mgr.GetBatchSize());
  for (int iter = 0; iter < num_iter; ++iter) {
    // Create Minibatch 
    for (int d_idx = 0; d_idx < workload_mgr.GetBatchSize(); ++d_idx) {
      int32_t data_idx = workload_mgr.GetDataIdxAndAdvance();
      Datum* datum = train_data_.datum(data_idx + minibatch_idx);
      SampleNegEntities(datum);

      minibatch[d_idx] = datum;
    }
    
    eeel_solver.Solve(minibatch);
    
    // Clear neg samples and related grads
    for (int d_idx = 0; d_idx < workload_mgr.GetBatchSize(); ++d_idx){
      // @hzt ??
      //Datum* Datum_to_clear = train_data_.datum(data_idx + minibatch_idx);
      //Datum_to_clear->clear_negs();
      minibatch[d_idx]->clear_negs();
    }

    workload_mgr.IncreaseDataIdxByBatchSize();
    
    if (workload_mgr.IsEndOfBatch()){
      ++num_batch_this_epoch;
      if (num_batch_this_epoch % num_batch_per_eval == 0) {
        //std::vector<int32_t> eval_data_idx = workload_mgr.GetBatchDataIdx(44);
        ++eval_counter;
        if (client_id == 0 && thread_id == 0) {
          //TODO
        }
      }
    }// end of batch
    break; //TODO: just for debug
  } // end of training
}

}  // namespace entity
