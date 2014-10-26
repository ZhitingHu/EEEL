#include "context.hpp"
//#include <gflags/gflags.h>
//#include <glog/logging.h>
#include <vector>

namespace util {

Context& Context::get_instance()
{
  static Context instance;
  return instance;
}

Context::Context() {
  //std::vector<google::CommandLineFlagInfo> flags;
  //google::GetAllFlags(&flags);
  //for (size_t i = 0; i < flags.size(); i++) {
  //  google::CommandLineFlagInfo& flag = flags[i];
  //  ctx_[flag.name] = flag.is_default ? flag.default_value : flag.current_value;
  //}
  dim_entity_vector_ = 100;
  distance_metric_mode_ = 0;
  dataset_path_ = "";
  num_epochs_ = 1;
  batch_size_ = 50;
  num_neg_sample_ = 10;
  learning_rate_ = 0.1;
  num_batches_per_eval_ = 10;
}

// -------------------- Getters ----------------------

int Context::get_int32(std::string key) {
  return atoi(get_string(key).c_str());
}

double Context::get_double(std::string key) {
  return atof(get_string(key).c_str());
}

bool Context::get_bool(std::string key) {
  return get_string(key).compare("true") == 0;
}

std::string Context::get_string(std::string key) {
  auto it = ctx_.find(key);
  //LOG_IF(FATAL, it == ctx_.end())
  //    << "Failed to lookup " << key << " in context.";
  if (it == ctx_.end()) {
    std::cerr << "Failed to lookup " << key << " in context." << std::endl;
    exit(0);
  }
  return it->second;
}

// -------------------- Setters ---------------------

void Context::set(std::string key, int value) {
  ctx_[key] = std::to_string((long long int)value);
}

void Context::set(std::string key, double value) {
  ctx_[key] = std::to_string((long double)value);
}

void Context::set(std::string key, bool value) {
  ctx_[key] = (value) ? "true" : "false";
}

void Context::set(std::string key, std::string value) {
  ctx_[key] = value;
}

}   // namespace util
