#ifndef ENTITY_NODE_HPP_
#define ENTITY_NODE_HPP_

#include "path.hpp"
#include <vector>
#include <cstdint>
#include <utility>

namespace entity {

class Node {
public:
  Node(const int id, const int level) : id_(id), level_(level) {};
  ~Node() {};
  
  void set_level(const int level){ level_ = level; };
  void AddParant(int p_idx);
  
  void AddChild(int c_idx);
  
  inline const vector<int>& parent_idx() { return parent_idx_; }
  inline const vector<int>& child_idx() { return child_idx_; }
  inline const int id() const { return id_; }
  inline const int level() const { return level_; }
  
  bool operator<(const Node& node) {
    return level_ < node.level();
  }

private: 
  // entity or category id
  int id_;  
  int level_;

  // parents' index in hierarchy
  vector<int> parent_idx_;
  // children's index in hierarchy
  vector<int> child_idx_;
};

}  // namespace entity

#endif
