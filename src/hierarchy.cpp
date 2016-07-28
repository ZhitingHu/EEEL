#include "hierarchy.hpp"
#include <vector>
#include <queue>
#include <stdint.h>
#include <utility>
#include <math.h>
#include <time.h>
#include <algorithm>

namespace entity {

/**
 * TODO: the implementation of the path-finding method is incomplete.
 *   See the paper for the correct method
 */

Path* Hierarchy::FindPathBetweenEntities(int entity_from, int entity_to) {
  set<int> common_ancestors;
  
  FindCommonAncestors(entity_from, entity_to, common_ancestors); 

  int num_ca = common_ancestors.size();
#ifdef DEBUG
  if (num_ca <= 0) {
    const map<int, float>& entity_from_ancestor_weights 
        = *entity_ancestor_weights_[entity_from]; 
    const map<int, float>& entity_to_ancestor_weights 
        = *entity_ancestor_weights_[entity_to];
    LOG(INFO) << "ENTITY FROM " << entity_from;
    map<int, float>::const_iterator it = entity_from_ancestor_weights.begin();    
    for (; it != entity_from_ancestor_weights.end(); ++it) {
      LOG(INFO) << it->first << "\t" << it->second;
    }
    LOG(INFO) << "ENTITY TO " << entity_to;
    it = entity_to_ancestor_weights.begin();    
    for (; it != entity_to_ancestor_weights.end(); ++it) {
      LOG(INFO) << it->first << "\t" << it->second;
    }
  }
  CHECK_GT(num_ca, 0);
#endif

  //t_start = clock(); 

  Path* path = new Path();
  float weight_sum = 0;
  //float ca_weight_sum = 0;
  map<int, float> ca_weights;

  ExpandPathFromCommonAncestors(entity_from, common_ancestors, 
      path, weight_sum, ca_weights);
  ExpandPathFromCommonAncestors(entity_to, common_ancestors, 
      path, weight_sum, ca_weights);

#ifdef DEBUG
  CHECK_GT(weight_sum, 0);
  CHECK_EQ(ca_weights.size(), common_ancestors.size());
#endif
  //path->ScaleCategoryWeights(1.0 / (weight_sum * /* sqrt(num_ca) */ ca_weight_sum)); 
  float max_ca_weight = 0;
  map<int, float>::const_iterator it = ca_weights.begin();
  for (; it != ca_weights.end(); ++it) {
    max_ca_weight = max(it->second, max_ca_weight);
  }
  path->ScaleCategoryWeights(1.0 / weight_sum, 1.0 / max_ca_weight); 

  return path;
}


void Hierarchy::ExpandPathFromCommonAncestors(const int entity_idx,
    const set<int>& common_ancestors, Path* path, float& weight_sum,
    map<int, float>& ca_weights) {

  const map<int, float>& entity_ancestor_weights 
      = *entity_ancestor_weights_[entity_idx];
  const map<int, vector<int> >& entity_ancestor_hierarchy
      = *entity_ancestor_hierarchies_[entity_idx];

  map<int, vector<int> >::const_iterator hierarchy_it;
  queue<int> unprocessed_nodes;
  set<int> processed_nodes;
  int cur_idx;
  float cur_weight;

  // expand from common ancestors
  set<int>::const_iterator set_it = common_ancestors.begin();
  for (; set_it != common_ancestors.end(); ++set_it) {
    //ca_weight_sum += entity_ancestor_weights.find(*set_it)->second;
    ca_weights[*set_it] += entity_ancestor_weights.find(*set_it)->second;
    unprocessed_nodes.push(*set_it);
  }
  while (!unprocessed_nodes.empty()) {
    cur_idx = unprocessed_nodes.front();
    unprocessed_nodes.pop();
    // check if being processed, 'cause it is a DAG
    if (processed_nodes.find(cur_idx) != processed_nodes.end()) {
      continue;
    } 
    processed_nodes.insert(cur_idx);
#ifdef DEBUG
    CHECK(entity_ancestor_weights.find(cur_idx) 
        != entity_ancestor_weights.end());
#endif
    cur_weight = entity_ancestor_weights.find(cur_idx)->second;
    path->AddCategoryNode(nodes_[cur_idx]->id(),
        cur_weight);
    weight_sum += cur_weight;

    hierarchy_it = entity_ancestor_hierarchy.find(cur_idx);
    if (hierarchy_it == entity_ancestor_hierarchy.end()) {
      continue;
    }
    const vector<int>& cur_children  = hierarchy_it->second;
    for (int c_idx_i = 0; c_idx_i < cur_children.size(); ++c_idx_i) {
      const int c_idx = cur_children[c_idx_i];
      unprocessed_nodes.push(cur_children[c_idx_i]);
    }
  } // end of expansion
}

/**
 * @param [IN] entity_from
 * @param [IN] entity_to
 * @param [OUT] common_ancestors:
 *     common ancestors that have disjoint paths to entity_from 
 *     and entity_to
 *     
 */
void Hierarchy::FindCommonAncestors(int entity_from, int entity_to,
    set<int>& common_ancestors) {
  // for speedup
  if (entity_ancestor_weights_[entity_from]->size() 
      < entity_ancestor_weights_[entity_to]->size()) {
    int tmp = entity_to;
    entity_to = entity_from;
    entity_from = tmp;
  }
  const map<int, float>& entity_from_ancestor_weights 
      = *entity_ancestor_weights_[entity_from]; 
  const map<int, float>& entity_to_ancestor_weights 
      = *entity_ancestor_weights_[entity_to]; 
  /// get common ancestors 
  int cur_ancestor_idx;
  int c_idx_i;
  map<int, float>::const_iterator map_cit_ = entity_to_ancestor_weights.begin();
  for (; map_cit_ != entity_to_ancestor_weights.end(); ++map_cit_) {
    cur_ancestor_idx = map_cit_->first;
    // common ancestor: have disjoint paths to entity_from and entity_to
    if (entity_from_ancestor_weights.find(cur_ancestor_idx) 
        != entity_from_ancestor_weights.end() && // an ancestor of entity_from
        common_ancestors.find(cur_ancestor_idx)  // make sure no duplicate
        == common_ancestors.end()) {
      const vector<int>& cur_ancestor_children 
          = nodes_[cur_ancestor_idx]->child_idx();
      // three features of children nodes
      //bool has_ancestor_of_from = false;
      int num_ancestor_of_from_or_to = 0;
      bool has_nonca_or_in_ca_set = false; 
      for (c_idx_i = 0; c_idx_i < cur_ancestor_children.size(); ++c_idx_i) {
        int c_idx = cur_ancestor_children[c_idx_i];
        if (c_idx == entity_from || entity_from_ancestor_weights.find(c_idx) 
            != entity_from_ancestor_weights.end()) {
          //has_ancestor_of_from = true;
          ++num_ancestor_of_from_or_to;
          if (entity_to_ancestor_weights.find(c_idx) 
              == entity_to_ancestor_weights.end()) {
            has_nonca_or_in_ca_set = true;
          }
        }
        if (c_idx == entity_to || entity_to_ancestor_weights.find(c_idx) 
            != entity_to_ancestor_weights.end()) {
          ++num_ancestor_of_from_or_to;
          if (entity_from_ancestor_weights.find(c_idx) 
              == entity_from_ancestor_weights.end()) {
            has_nonca_or_in_ca_set = true;
          }
        }
        if (common_ancestors.find(c_idx) != common_ancestors.end()) {
          has_nonca_or_in_ca_set = true;
        } 
        // check if conditions are satisfied 
        if (/*has_ancestor_of_from &&*/ has_nonca_or_in_ca_set 
            && num_ancestor_of_from_or_to > 1) {
          common_ancestors.insert(cur_ancestor_idx);
          break;
        }
      } // end of traversing cur_ancestor's children
    }
  } // end of traversing entity_to's ancestors 
}

}  // namespace entity
