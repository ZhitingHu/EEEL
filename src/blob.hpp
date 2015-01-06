#ifndef ENTITY_BLOB_HPP_
#define ENTITY_BLOB_HPP_

#include "common.hpp"

#include <vector>
#include <utility>
#include <string>
#include <cmath>

namespace entity {

class Blob {
public:
  Blob(const int num_row, const int num_col = 1) 
      : num_row_(num_row), num_col_(num_col) { 
    if (num_col == 1 
        || entity::Context::dist_metric_mode() == entity::Context::DIAG) {
      count_ = num_row_;
    } else if (entity::Context::dist_metric_mode() == entity::Context::EDIAG) {
      count_ = 1;
    } else if (entity::Context::dist_metric_mode() == entity::Context::FULL) {
      count_ = num_row_ * num_col_;
    } else {
      LOG(FATAL) << "Unknown metric mode.";
    }
    data_ = new float[count_];
    ClearData();
  };

  ~Blob() { delete data_; };
  
  // read only 
  const float* data() const { return data_; }
  // mutable
  float* mutable_data() { return data_; }

  inline int count() const { return count_; }
  inline int num_row() const { return num_row_; }
  inline int num_col() const { return num_col_; }

  inline float data_at(const int r, const int c = 0) const {
#ifdef DEBUG
    CHECK_LT(r, num_row_);
    CHECK_LT(c, num_col_);
#endif
    return *(data() + offset(r, c));
  }
  
  void init_data_at(const float init_val, const int r, const int c = 0) {
#ifdef DEBUG
    CHECK_LT(r, num_row_);
    CHECK_LT(c, num_col_);
#endif
    *(mutable_data() + offset(r, c)) = init_val;   
  }
  
  inline void ClearData() {
    memset(data_, 0, count_ * sizeof(float));
  }
  
  void CopyFrom(const Blob* source, const float coeff = 1.0) {
#ifdef DEBUG
    CHECK_EQ(count_, source->count());
#endif
    //memcpy(data_, source->data(), sizeof(float) * count_);
    const float* source_data = source->data();
    for (int idx = 0; idx < count_; ++idx) {
      data_[idx] = source_data[idx] * coeff;
    }  
  }

  void Accumulate(const Blob* source, const float coeff = 1.0) {
#ifdef DEBUG
    CHECK_EQ(count_, source->count());
#endif
    const float* source_data = source->data();
    for (int idx = 0; idx < count_; ++idx) {
      data_[idx] += source_data[idx] * coeff;
#ifdef DEBUG
    CHECK(!isnan(data_[idx]));
#endif
    }  
  }

  void Normalize() {
#ifdef DEBUG
    CHECK_EQ(num_col_, 1);
#endif
    float sum = 0;
    for (int idx = 0; idx < count_; ++idx) {
      sum += data_[idx] * data_[idx];
    }
    sum = sqrt(sum);
#ifdef DEBUG
    if (!(sum > 1e-7)) {
      for (int idx = 0; idx < count_; ++idx) {
        //sum += data_[idx] * data_[idx];
        LOG(ERROR) << data_[idx];
      }
    }
    CHECK_GT(sum, 1e-7);
#endif
    for (int idx = 0; idx < count_; ++idx) {
      data_[idx] /= sum;
    }
  }
  
  void Rectify() {
    for (int idx = 0; idx < count_; ++idx) {
      data_[idx] = (data_[idx] > 0 ? data_[idx] : 0);
    }
  }

  // test 
  void CheckNaN() {
    for (int idx = 0; idx < count_; ++idx) {
      CHECK(!isnan(data_[idx]));
    }
  }

private:

  inline int offset(const int r, const int c) const {
    if (num_col_ == 1 
        || entity::Context::dist_metric_mode() == entity::Context::DIAG) {
      return r;
    } else if (entity::Context::dist_metric_mode() == entity::Context::EDIAG) {
      return 0;
    } else if (entity::Context::dist_metric_mode() == entity::Context::FULL) {
      return r * num_col_ + c;
    } else {
      LOG(FATAL) << "Unknown metric mode.";
    }
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
};

}  // namespace entity

#endif
