#ifndef ENTITY_HIERARCHY_HPP_
#define ENTITY_HIERARCHY_HPP_

#include "path.hpp"
#include "node.hpp"
#include <vector>
#include <map>
#include <cstdint>
#include <utility>

namespace entity {

class Hierarchy {
public:
  Hierarchy() {};
  ~Hierarchy() {};
 
  Path* FindPathBetweenEntities(int entity_from, int entity_to);
    
private:
  // @return: an sorted array of ancestor nodes
  //     sorted according to node level
  const vector<Node*>& FindCommonAncestors(int entity_from, int entity_to);

private:
  // [0, num_entity): entity nodes
  // [num_entity, num_entity + num_category): category nodes
  // Note: entity_id = index in nodes_;
  //       category_id = index in nodes_ - num_entity
  vector<Node*> nodes_;

  int num_entity_;
  int num_category_;
  int max_node_level_;
};

}  // namespace entity

#endif
