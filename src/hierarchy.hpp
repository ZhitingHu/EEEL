#ifndef ENTITY_HIERARCHY_HPP_
#define ENTITY_HIERARCHY_HPP_

#include "path.hpp"
#include "node.hpp"
#include <vector>
#include <map>
#include <set>
#include <stdint.h>
#include <utility>

namespace entity {

class Hierarchy {
public:
  Hierarchy() {};
  ~Hierarchy() {};
 
  Path* FindPathBetweenEntities(int entity_from, int entity_to);
 
  void InitHierarchy(int num_entity, int num_category){
    Node *node; 
    // entity node
    for (int nidx = 0; nidx < num_entity; ++nidx){
      node = new Node(nidx, -1);  // init levels of entities to -1 
      nodes_.push_back(node);
    }
    // category node
    for (int nidx = 0; nidx < num_category; ++nidx){
      node = new Node(nidx, -1);
      nodes_.push_back(node);
    }
    entity_ancestor_weights_.resize(num_entity);
    entity_ancestor_hierarchies_.resize(num_entity);
  };
  
  //void SetNodeLevel(int idx, int level){
  //  nodes_[idx]->set_level(level);
  //};

  Node* node(int idx) { return nodes_[idx]; }


  void AddAncestorWeights(const int entity_id, 
      map<int, float>* ancestor_weight_map) {
    entity_ancestor_weights_[entity_id] = ancestor_weight_map;

    // build entity's ancestor hierarchy
    map<int, vector<int> >* ancestor_hierarchy = new map<int, vector<int> >;
    map<int, float>::const_iterator it = ancestor_weight_map->begin();
    for (; it != ancestor_weight_map->end(); ++it) {
      const vector<int>& parents = nodes_[it->first]->parent_idx();
      for (int p_idx_i = 0; p_idx_i < parents.size(); ++p_idx_i) {
        const int p_idx = parents[p_idx_i];
        if (ancestor_weight_map->find(p_idx) != ancestor_weight_map->end()) {
          (*ancestor_hierarchy)[p_idx].push_back(it->first);
        }
      }
    }
   
    //map<int, vector<int>* >* ancestor_hierarchy = new map<int, vector<int>* >;
    //map<int, float>::const_iterator it = ancestor_weight_map->begin();
    //for (; it != ancestor_weight_map->end(); ++it) {
    //  vector<int>* 
    //  const vector<int>& all_children = nodes_[it->first]->child_idx();
    //  for (int c_idx_i = 0; c_idx_i < all_children.size(); ++c_idx_i) {
    //    const int child_idx = all_children[c_idx_i];
    //    if (ancestor_weight_map->find(child_idx) != ancestor_weight_map->end()){
    //     
    //    }
    //  }
    //  
    //}

    entity_ancestor_hierarchies_[entity_id] = ancestor_hierarchy;
  }

private:
  void FindCommonAncestors(int entity_from, int entity_to,
      set<int>& common_ancestors);

  //void FindCommonAncestors(const int entity_from, const int entity_to,
  //    map<int, int>& entity_from_ancestor_weights, 
  //    map<int, int>& entity_to_ancestor_weights,
  //    set<int>& common_ancestors);

  void ExpandPathFromCommonAncestors(const int entity_idx,
      const set<int>& common_ancestors, Path* path, float& weight_sum,
      float& ca_weight_sum);

private:
  // [0, num_entity): entity nodes
  // [num_entity, num_entity + num_category): category nodes
  // Note: entity_id = index in nodes_;
  //       category_id = index in nodes_ - num_entity
  vector<Node*> nodes_;
 
  // dim = num_entity
  // each entry map<int, float> is: ancestor_category_idx => weight
  vector<map<int, float>* > entity_ancestor_weights_;
  // dim = num_entity
  // each entry map<int, vector<int> > is: 
  // ancestor_category_idx => child category idx of entity's ancestor
  vector<map<int, vector<int> >* > entity_ancestor_hierarchies_; 

  int num_entity_;
  int num_category_;
  int max_node_level_;

  //set<int>::const_iterator set_it_;
  //map<int, float>::const_iterator map_cit_;
};

}  // namespace entity

#endif
