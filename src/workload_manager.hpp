#pragma once

//#include <glog/logging.h>
#include <cstdint>


namespace entity {

struct WorkloadManagerConfig {
  int32_t thread_id;
  int32_t client_id;
  int32_t num_clients;
  int32_t num_threads;
  //int32_t num_batches_per_epoch;
  int32_t batch_size;
  int32_t num_data;
};

class WorkloadManager {
public:
  WorkloadManager(const WorkloadManagerConfig& config) {
    int32_t thread_id = config.thread_id;
    int32_t client_id = config.client_id;
    int32_t num_clients = config.num_clients;
    int32_t num_threads = config.num_threads;
    //int32_t num_batches_per_epoch = config.num_batches_per_epoch;

    // Each thread handles data [data_idx_begin_, data_idx_end_).
    int num_data = config.num_data;
    int num_data_per_thread = num_data / (num_clients * num_threads);
    data_idx_begin_ = num_data_per_thread *
      (client_id * num_threads + thread_id);
    // The last thread takes the rest of the data.
    if (client_id == num_clients - 1 && thread_id == num_threads - 1) {
      data_idx_end_ = num_data;
    } else {
      data_idx_end_ = data_idx_begin_ + num_data_per_thread;
    }
    //batch_size_ = (data_idx_end_ - data_idx_begin_) / num_batches_per_epoch;
    batch_size_ = config.batch_size;
    CHECK_GT(batch_size_, 0);
    Restart();
  }

  int32_t GetBatchSize() const {
    return batch_size_;
  }

  void inline Restart() {
    curr_data_idx_ = data_idx_begin_;
  }

  // Get a data index and advance.
  int32_t inline GetDataIdxAndAdvance() {
#ifdef DEBUG
    CHECK(!IsEnd());
#endif
    int ret = curr_data_idx_;
    ++curr_data_idx_;
    if (IsEnd()) {
      Restart();
    }
    return ret;
  }

  //
  void inline IncreaseDataIdxByBatchSize() {
#ifdef DEBUG
    CHECK(!IsEnd());
#endif
    curr_data_idx_ += batch_size_;
    if (curr_data_idx_ >= data_idx_end_) {
      curr_data_idx_ = (curr_data_idx_ - data_idx_end_)
        % (data_idx_end_ - data_idx_begin_) + data_idx_begin_;
    }
  }

  // Get the next num_data indices without advancing.
  void GetBatchDataIdx(const int num_data, vector<int>& batch_data_idx) const {
    for (int i = 0; i < num_data; ++i) {
      batch_data_idx[i] = curr_data_idx_ + i;
      // Wrap around to be within [data_idx_begin_, data_idx_end_)
      if (batch_data_idx[i] >= data_idx_end_) {
        batch_data_idx[i] = (batch_data_idx[i] - data_idx_end_)
          % (data_idx_end_ - data_idx_begin_) + data_idx_begin_;
      }
    }
  }

  // Is end of the data set (of this partition).
  bool inline IsEnd() {
    return curr_data_idx_ == data_idx_end_;
  }

  bool inline IsEndOfBatch() const {
    return (curr_data_idx_ - data_idx_begin_) % batch_size_ == 0;
  }

private:
  // A partition's index range is [data_idx_begin_, data_idx_end_).
  int32_t data_idx_begin_;
  int32_t data_idx_end_;

  // # of data points to go through before IsEndOfBatch() returns true.
  int32_t batch_size_;
  int32_t curr_data_idx_;
};


}  // namespace entity


