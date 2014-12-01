#include "node.hpp"
#include <vector>
#include <cstdint>
#include <utility>

namespace entity {

void Node::AddParent(int p_idx) { 
//#ifdef DEBUG
//  for (int i = 0; i < parent_idx_.size(); ++i) {
//    CHECK(parent_idx_[i] !=  p_idx);
//  }
//#endif
  for (int i = 0; i < parent_idx_.size(); ++i) {
    if (parent_idx_[i] ==  p_idx) { return; }
  }
  parent_idx_.push_back(p_idx);
} 
  
void Node::AddChild(int c_idx) { 
//#ifdef DEBUG
//  for (int i = 0; i < child_idx_.size(); ++i) {
//    CHECK(child_idx_[i] !=  c_idx);
//  }
//#endif
  for (int i = 0; i < child_idx_.size(); ++i) {
    if (child_idx_[i] ==  c_idx) { return; }
  }
  child_idx_.push_back(c_idx);
} 

}  // namespace entity
