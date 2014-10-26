#pragma once

#include "common.hpp"
//#include <gflags/gflags.h>
//#include <glog/logging.h>
#include <vector>
#include <string>
#include <unordered_map>

namespace util {

// An extension of google flags. It is a singleton that stores 1) google flags
// and 2) other lightweight global flags. Underlying data structure is map of
// string and string, similar to google::CommandLineFlagInfo.
class Context {
public:
  static Context& get_instance();

  int get_int32(std::string key);
  double get_double(std::string key);
  bool get_bool(std::string key);
  std::string get_string(std::string key);

  void set(std::string key, int value);
  void set(std::string key, double value);
  void set(std::string key, bool value);
  void set(std::string key, std::string value);
  
  const int get_dim_entity_vector() { return dim_entity_vector_; }
  const int get_distance_metric_mode() { return distance_metric_mode_; }
  const std::string& get_dataset_path() { return dataset_path_; }
  const int get_num_epochs() { return num_epochs_; }
  const int get_batch_size() { return batch_size_; }
  const int get_num_neg_sample() { return num_neg_sample_; }
  const double get_learning_rate() { return learning_rate_; }
  const int get_num_batches_per_eval() { return num_batches_per_eval_; }
  
 private:
  
  // entity embedding parameters
  int dim_entity_vector_; // 100
  int distance_metric_mode_; //
  std::string dataset_path_; //
  int num_epochs_; // 1
  int batch_size_; // 50
  int num_neg_sample_; // 50
  double learning_rate_; // 0.1, Initial step size
  int num_batches_per_eval_; // 10, Number of batches per evaluation

  // Private constructor. Store all the gflags values.
  Context();
  // Underlying data structure
  std::unordered_map<std::string, std::string> ctx_;
};

}   // namespace util
