#include "context.hpp"
#include <vector>
#include <string>
#pragma comment(lib,"libglog.lib")  
#pragma comment(lib,"gflags.lib")  
#pragma comment(lib,"gflags_nothreads.lib")	

using namespace gflags;

namespace entity {

Context& Context::get_instance()
{
  static Context instance;
  return instance;
}

Context::Context() {
  std::vector<CommandLineFlagInfo> flags;
  GetAllFlags(&flags);
  for (size_t i = 0; i < flags.size(); ++i) {
    CommandLineFlagInfo& flag = flags[i];
    ctx_[flag.name] = flag.is_default ? flag.default_value : flag.current_value;
  }
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
  LOG_IF(FATAL, it == ctx_.end())
      << "Failed to lookup " << key << " in context.";
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

void Context::set_dist_metric_mode() {
  string dist_metric_mode_str = get_string("distance_metric_mode");
  if (dist_metric_mode_str == "FULL") {
    dist_metric_mode_ = DistMetricMode::FULL;
  } else if (dist_metric_mode_str == "DIAG") {
    dist_metric_mode_ = DistMetricMode::DIAG;
  } else if (dist_metric_mode_str == "EDIAG") {
    dist_metric_mode_ = DistMetricMode::EDIAG;
  } else {
    LOG(FATAL) << "Unkown Distance_Metric_Mode";
  }
}

}   // namespace entity 
