#pragma once

#include "solver.hpp"
#include "dataset.hpp"
#include "context.hpp"
#include <vector>
#include <cstdint>
#include <utility>

namespace entity {

class EEEngine {
public:
  EEEngine() {};
  ~EEEngine() {};

  void ReadData();

  void Start();

  // Compute the classification error on the test set.
  //float TestClassificationError(BackPropSolver* solver) const;

private:    // private functions

private:

  // ============== EE Variables ==================
  int num_train_data_;

  Dataset train_data_;
};

}  // namespace entity
