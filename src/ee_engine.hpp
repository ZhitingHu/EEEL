// Author: Zhi-Ting Hu, Po-Yao Huang
// Date: 2014.10.26
#pragma once

#include "solver.hpp"
#include "common.hpp"
#include "dataset.hpp"
#include "context.hpp"
#include "node.hpp"
#include "hierarchy.hpp"
#include <vector>
#include <stdint.h>
#include <utility>
#include <iostream>
#include <fstream>

namespace entity {

class EEEngine {
public:
  EEEngine();
  ~EEEngine();

  void ReadData();

  void Start();
 
  // for analysis
  Hierarchy* entity_category_hierarchy() { 
    return &entity_category_hierarchy_; 
  }

private:    // private functions

  void SampleNegEntities(Datum* datum);
  void ThreadCreateMinibatch(const vector<int>* next_minibatch_data_idx,
      vector<Datum*>* next_minibatch);

  inline void CopyMinibatch(const vector<Datum*>& source, 
      vector<Datum*>& target); 
  inline void ClearMinibatch(vector<Datum*>& minibatch);
  
private:

  Dataset train_data_;
  //Dataset test_data_;
  int num_train_data_;  

  Hierarchy entity_category_hierarchy_;

  int32_t num_neg_sample_;
  int32_t num_entity_;
  int32_t num_category_;
};

}  // namespace entity
