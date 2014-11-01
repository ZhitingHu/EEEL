#ifndef UTIL_CONTEXT_HPP_
#define UTIL_CONTEXT_HPP_

#include "common.hpp"
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <vector>
#include <string>
#include <unordered_map>

namespace entity {

// A singleton that stores 1) google flags and 
// 2) other lightweight global flags.
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
 
  enum DistMetricMode {
    DIAG, // diagonal matrix
    EDIAG, // diagonal matirx with tied diagonal elements
    FULL // full PSD matrix
  };

  inline static const DistMetricMode dist_metric_mode() { 
    return get_instance().dist_metric_mode_; 
  }
  inline static const int dim_embedding() { 
    return get_instance().dim_embedding_; 
  }
 
private:
  void set_dist_metric_mode();  

private:
  // Private constructor. Store all the gflags values.
  Context();
  // Underlying data structure
  std::unordered_map<std::string, std::string> ctx_;


  DistMetricMode dist_metric_mode_;
  int dim_embedding_;
};

} // namespace entity

#endif // UTIL_CONTEXT_HPP_ 
