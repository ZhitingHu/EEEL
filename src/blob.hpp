#pragma once

#include "path.hpp"
#include <vector>
#include <cstdint>
#include <utility>

namespace entity {

class Blob {
public:
  Blob(const int num_row, const int num_col = 1) : num_row_(num_row), num_col_(num_col), 
      count_(num_row * num_col), matrix_(false) { 
    data_ = new float[count_]; 
  };

  ~Blob() { delete data_; };
  
  // read only 
  const float* data() const { return data_; }
  // mutable
  float* mutable_data() { return data_; }

  // write data // add by bernie // to be discussed
  void set_data(const int i, const float value) { data_[i] = value; }
  
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
