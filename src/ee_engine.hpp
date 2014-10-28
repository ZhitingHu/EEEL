// Author: Zhi-Ting Hu, Po-Yao Huang
// Date: 2014.10.26
#pragma once

#include "solver.hpp"
#include "common.hpp"
#include "dataset.hpp"
#include "context.hpp"
#include <vector>
#include <cstdint>
#include <utility>

namespace entity {

class EEEngine {
public:
  EEEngine();
  ~EEEngine();

  void ReadData();

  void Start();

private:    // private functions

	private:

  // ============== EE Variables ==================
  int32_t num_train_data_;
  int32_t num_test_data_;

  Dataset train_data_;
};

}  // namespace entity
