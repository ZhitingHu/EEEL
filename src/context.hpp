#pragma once
#include <gflags/gflags.h>
#include <glog/logging.h>
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

 private:
  
// entity embedding parameters
int dim_entity_vector; //100
int distance_metric_mode; //
string dataset_path; //
int num_epochs; // 1
DEFINE_int32(batch_size, 50, "");
DEFINE_int32(num_neg_sample, 50, "");
DEFINE_double(learning_rate, 0.1, "Initial step size");
DEFINE_int32(num_batches_per_eval, 10, "Number of batches per evaluation");

  // Private constructor. Store all the gflags values.
  Context();
  // Underlying data structure
  std::unordered_map<std::string, std::string> ctx_;
};

}   // namespace util
