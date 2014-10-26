#pragma once

#include "ee_solver.hpp"
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

  // Can be called concurrently.
  void Start();

  // Compute the classification error on the test set.
  //float TestClassificationError(BackPropSolver* solver) const;

private:    // private functions

private:

  // ============== EE Variables ==================
  int num_train_data_;

  std::vector<Datum*> train_data_;
};

}  // namespace entity
