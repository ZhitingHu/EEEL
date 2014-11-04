#include "node.hpp"
#include <vector>
#include <cstdint>
#include <utility>

namespace entity {

void Node::AddParant(int p_idx) { 
#ifdef DEBUG
  for (int i = 0; i < parent_idx_.size(); ++i) {
    CHECK(parent_idx_[i] !=  p_idx);
  }
#endif
  parent_idx_.push_back(p_idx);
} 
  
void Node::AddChild(int c_idx) { 
#ifdef DEBUG
  for (int i = 0; i < child_idx_.size(); ++i) {
    CHECK(child_idx_[i] !=  c_idx);
  }
#endif
  child_idx_.push_back(c_idx);
} 

}  // namespace entity
