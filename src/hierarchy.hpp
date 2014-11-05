#ifndef ENTITY_HIERARCHY_HPP_
#define ENTITY_HIERARCHY_HPP_

#include "path.hpp"
#include "node.hpp"
#include <vector>
#include <map>
#include <set>
#include <cstdint>
#include <utility>

namespace entity {

class Hierarchy {
public:
  Hierarchy() {};
  ~Hierarchy() {};
 
  Path* FindPathBetweenEntities(int entity_from, int entity_to);
 
  void Init_Hierarchy(int size){
    Node *Node_ptr; //= static_cast<Node*>(::operator new (sizeof(Node)*size));
    //vector<Node*> 
    for (int Node_idx = 0; Node_idx < size; ++Node_idx){
      Node_ptr = new Node(Node_idx, -1);  // init levels of entities to -1 
      nodes_.push_back(Node_ptr);
    }
  };
  
  void Set_Node_level(int id, int level){
    nodes_[id]->set_level(level);
  };

  Node* Get_Node_adr(int idx) const { return (Node*) nodes_[idx]; }


  void Add_weights(map<int, float> map_weight){
    entity_ancestor_weights_.push_back(map_weight);
  }

private:
  void FindCommonAncestors(int entity_from, int entity_to,
      set<int>& common_ancestors);

  //void FindCommonAncestors(const int entity_from, const int entity_to,
  //    map<int, int>& entity_from_ancestor_weights, 
  //    map<int, int>& entity_to_ancestor_weights,
  //    set<int>& common_ancestors);

private:
  // [0, num_entity): entity nodes
  // [num_entity, num_entity + num_category): category nodes
  // Note: entity_id = index in nodes_;
  //       category_id = index in nodes_ - num_entity
  vector<Node*> nodes_;
 
  // dim = num_entity
  // Each entry map<int, float> is ancestor_category_id => weight
  vector<map<int, float> > entity_ancestor_weights_;

  int num_entity_;
  int num_category_;
  int max_node_level_;

  set<int>::const_iterator set_it_;
  map<int, float>::const_iterator map_cit_;
};

}  // namespace entity

#endif
