// Author: Zhi-Ting Hu, Po-Yao Huang
// Date: 2014.10.26
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "context.hpp"
#include "ee_engine.hpp"
#include "analyst.hpp"
#include "solver.hpp"
#include "blob.hpp"

#include <thread>
#include <vector>
#include <cstdint>
#include <iostream>
#include <string>

using namespace std;

// (temp) entity embedding parameters
DEFINE_int32(dim_embedding, 100, "");
DEFINE_string(distance_metric_mode, "DIAG", "");
DEFINE_int32(num_neg_sample, 50, "");

// Results
DEFINE_string(resume_path, "output/", "Results to be analyzed");
DEFINE_int32(resume_iter, 100, "Iteration of results");
// Data
DEFINE_string(dataset_path, "data/tech/", "data path");
DEFINE_string(output_file_prefix, "output/", "Results go here.");
DEFINE_string(category_filename, "categories.txt", "category filename");
DEFINE_string(entity_filename, "entity.txt", "entity filename");
//DEFINE_string(entity_to_ancestor_filename, "entity2ancestor.bin", "entity-ancestor filename");
DEFINE_string(entity_to_ancestor_filename, "entity2ancestor.txt", "entity-ancestor filename");
DEFINE_string(entity_to_category_filename, "entity2category.txt", "entity-category filename");
DEFINE_string(hierarchy_filename, "hierarchy.txt", "hierarchy filename");
DEFINE_string(hierarchy_id_filename, "hierarchy_id.txt", "hierarchy id filename");
DEFINE_string(pair_filename, "pair.txt", "pair id filename");
DEFINE_string(level_filename, "level.txt", "category level filename");

void OutputEntityVectors() {
  entity::Solver solver;
  solver.Restore(FLAGS_resume_path, FLAGS_resume_iter);

  vector<int> entities = {6148, 24259, 11937, 16344, 754, 382476, 638358, 434};
  string filename = FLAGS_output_file_prefix + "/" + "example_entity_vecs.txt";
  ofstream output;
  output.open(filename.c_str());
  CHECK(output.is_open()) << "Fail to open " << filename;
  for (int i = 0; i < entities.size(); ++i) {
    const entity::Blob* entity_vec = solver.entities()[entities[i]];
    const float* vec = entity_vec->data();
    const int count = entity_vec->count();
    output << entities[i] << ":\t";
    for (int j = 0; j < count; ++j) {
        output << vec[j] << "\t";
    }
    output << "\n";
  }
  output.flush();
  output.close();
}

void InteractiveNNByCategroy(entity::Analyst& analyst) {
  string filename = FLAGS_output_file_prefix + "/" + "interactive_nn";
  ofstream output;
  output.open(filename.c_str());
  CHECK(output.is_open()) << "Fail to open " << filename;

  int entity_id;
  vector<int> categories;
  int category_id;
  string buffer;
  while (true) {
    // input entity id
    cout << "Input target entity id (end by Ctrl-C): " << endl;
    while (true) {
      cin >> buffer;
      stringstream ss(buffer);
      if (ss >> entity_id) {
        break;
      }
      cout << "Invalid entity id. Re-input" << endl;
    }
    // input category ids
    cout << "Input category ids, end by -1" << endl;
    while (cin >> buffer) {
      stringstream ss(buffer);
      if (ss >> category_id) {
        if (category_id == -1) {
          cout << "Done. entity_id=" << entity_id << " #cate=" 
              << categories.size() << endl;
          break; 
        } else {
          categories.push_back(category_id);
        }
      } else {
        cout << "Invalid input. Re-input" << endl;
      }
    }
    // Compute NN
    analyst.ComputeNearestNeiborsByCategories(
        30, entity_id, categories, output);
    categories.clear();
  }
  output.close();
}

int main(int argc, char *argv[]) {
  FLAGS_alsologtostderr = 1;
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  entity::Context::set_phase(entity::Context::ANALYZE);

  entity::Analyst analyst(FLAGS_resume_path, FLAGS_resume_iter);
  //InteractiveNNByCategroy(analyst);

  //vector<int> candidate_entities = {9336, 397504, 719040, 1480281, 131};
  //vector<int> candidate_entities = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 20};
  //vector<int> candidate_entities = {0, 1, 2, 3, 20};
  //analyst.ComputeNearestNeibors(10, candidate_entities, 
  //    FLAGS_output_file_prefix);
  
  vector<int> candidate_entities = {0, 20};
  vector<vector<vector<int> > > categories = {{{177},{178}}, {{231},{134},{16,177},{177}}};
  analyst.ComputeNearestNeiborsByCategories(10, candidate_entities, categories,
      FLAGS_output_file_prefix);

  //OutputEntityVectors();

  LOG(ERROR) << "Done."; 
 
  return 0;
}

