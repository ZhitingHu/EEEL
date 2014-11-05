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
  int num_category = 0;
  int num_entity = 0;

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
  
  // file streams
  ifstream category_file(dataset_path + category_filename);
  ifstream entity_file(dataset_path + entity_filename);
  ifstream entity_ancestor_file(dataset_path + entity_to_ancestor_filename);
  ifstream entity_category_file(dataset_path + entity_to_category_filename);
  ifstream hierarchy_file(dataset_path + hierarchy_filename);
  ifstream hierarchy_id_file(dataset_path + hierarchy_id_filename);
  ifstream pair_file(dataset_path + pair_filename);

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

  // 1. Parse Category File
  while (getline(category_file, line)){
    num_category++;
  }
  cout << "number of category: " << num_category << endl;
  
  // 2. Parse Entity File
  while (getline(entity_file, line)){
    num_entity_++;
  }
  cout << "number of entity: " << num_entity_ << endl;


  // init category hierarchy
  entuty_category_hierarchy_.Init_Hierarchy(num_category + num_entity_);

  // entity_id = index in nodes_;
  // category_id = index in nodes_ - num_entity

  // 3. Parse entity file
  while (getline(entity_category_file, line)){
    istringstream iss(line);
    vector<int> tokens{ istream_iterator<int>{iss}, istream_iterator<int>{} };
    
    // current entity
    Node *temp_entity_node = entuty_category_hierarchy_.Get_Node_adr(tokens[0]);

    // add parents categories to current entity
    for (int level = 1; level < tokens.size(); ++level)
      temp_entity_node->AddParant(tokens[level] + num_entity_);
  }

  // 4. Parse hierarchy_id (category)
  while (getline(hierarchy_id_file, line)){
    istringstream iss(line);
    vector<int> tokens{ istream_iterator<int>{iss}, istream_iterator<int>{}};

    for (int level = 0; level < tokens.size(); ++level){
      Node *temp_category_node = entuty_category_hierarchy_.Get_Node_adr(tokens[level] + num_entity_);

      // set node/level
      //temp_category_node->set_Node(tokens[level], level);

      // add child categories to current category
      if (level + 1 < tokens.size())
        temp_category_node->AddChild(tokens[level + 1] + num_entity_);

      // add parent categories to current category
      if (level > 0)
        temp_category_node->AddParant(tokens[level - 1] + num_entity_);
    }
  }
  
  
  // 5. Parse Entity-Ancestor File
  while (getline(entity_ancestor_file, line)){
    istringstream iss(line);
    vector<string> tokens{ istream_iterator<string>{iss}, istream_iterator<string>{} };
    //743:8.3
    // current entity
    Node *temp_entity_node = entuty_category_hierarchy_.Get_Node_adr(stoi(tokens[0]));

    // add Entity Ancestor 
    for (int level = 1; level < tokens.size(); ++level){
      vector<string> temp_string = split(tokens[level], ':');
      map<int, float> temp_map;
      
      temp_map.insert(pair<int, float>(stoi(temp_string[0]), stof(temp_string[1])));
      //cout << stoi(temp_string[0]) << "\t" << stof(temp_string[1]) << endl;
      entuty_category_hierarchy_.Add_weights(temp_map);
    }
  }

  // 6. Parse pair File
  // Remark: current memory allocation is not good, consider allocate once than assign in future version.
  while (getline(pair_file, line)){
    istringstream iss(line);
    vector<int> tokens{ istream_iterator<int>{iss},istream_iterator<int>{}};
    train_data_.Add_Datum(tokens[0], tokens[1], tokens[2]);

    // Add path to pairs
    Path* entity_pair_path = entuty_category_hierarchy_.FindPathBetweenEntities(tokens[0], tokens[1]);
    train_data_.Add_Path(num_train_data_, tokens[1], entity_pair_path);
    
    num_train_data_++;
    //Datum *temp = train_data_.Get_Datum_adr(num_train_data_ - 1);
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

}// readdata

void EEEngine::SampleNegEntities(const Datum* datum) {
  // merged in Start()
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
        
        // Extract Datum from Dataset and to minibatch  
        minibatch.push_back(train_data_.Get_Datum_adr(data_idx + minibatch_idx));
        int32_t entity_id = minibatch[minibatch_idx]->entity_i();

        // Negative Sampling
        for (int neg_sample_idx = 0; neg_sample_idx < num_neg_sample_; neg_sample_idx++){
          int32_t neg_entity_id = static_cast <int> (rand()) % static_cast <int> (num_entity_);

          //TODO: consider handle duplicated case
          while (neg_entity_id == entity_id)
            neg_entity_id = static_cast <int> (rand()) % static_cast <int> (num_entity_);
          //cout << neg_entity_id << " ";

          // Generate Path between entity_i and neg_sample
          Path *neg_entity_path;
          neg_entity_path = entuty_category_hierarchy_.FindPathBetweenEntities(entity_id, neg_entity_id);

          minibatch[minibatch_idx]->AddNegSample(neg_entity_id, neg_entity_path);
        }
      }// minibatch
      
      //eepl_solver.Solve(minibatch);
      
      // Clear neg samples and related grads
      for (int minibatch_idx = 0; minibatch_idx < workload_mgr.GetBatchSize(); ++minibatch_idx){
        
        // add Datum to minibatch from Dataset 
        Datum* Datum_to_clear = train_data_.Get_Datum_adr(data_idx + minibatch_idx);
        Datum_to_clear->clear_negs();
      }

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
