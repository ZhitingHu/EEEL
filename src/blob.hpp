#pragma once

#include "path.hpp"
#include <vector>
#include <cstdint>
#include <utility>

namespace entity {

class Blob {
public:
  Blob(const int num_row): num_row_(num_row), num_col_(1), count_(num_row),
      matrix_(false) {};
  ~Blob() {};
  
  // read only 
  const float* data() const { return data_; }
  // mutable
  float* mutable_data() { return data_; }

  inline int count() const { return count_; }
    
  inline int offset(const int r, const int c) const {
    //TODO: do some check
    
    return r * num_col_ + c;
  }
  inline float data_at(const int r, const int c = 0) const {
    return *(data() + offset(r, c));
  }
  
private: 
  float* data_;

  // first dimention 
  int num_row_;
  // second dimention
  // = 1 if data_ is a vector
  int num_col_;
  // number of elements
  int count_;
  
  bool matrix_;
};

}  // namespace entity
