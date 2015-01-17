#include "hierarchy.hpp"
#include <vector>
#include <queue>
#include <stdint.h>
#include <utility>
#include <math.h>
#include <time.h>
#include <algorithm>

namespace entity {

Path* Hierarchy::FindPathBetweenEntities(int entity_from, int entity_to) {
  set<int> common_ancestors;
  
  //clock_t t_start = clock();

  FindCommonAncestors(entity_from, entity_to, common_ancestors); 

  //LOG(INFO) << "find common ancestor: "
  //    << ((double)(clock() - t_start) / CLOCKS_PER_SEC);
  //find_time += ((double)(clock() - t_start) / CLOCKS_PER_SEC);

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

  //const map<int, float>& entity_from_ancestor_weights 
  //    = *entity_ancestor_weights_[entity_from]; 
  //const map<int, float>& entity_to_ancestor_weights 
  //    = *entity_ancestor_weights_[entity_to]; 
  //const map<int, vector<int> >& entity_from_ancestor_hierarchy 
  //    = *entity_ancestor_hierarchies_[entity_from];
  //const map<int, vector<int> >& entity_to_ancestor_hierarchy 
  //    = *entity_ancestor_hierarchies_[entity_to];
  //const map<int, float>& entity_from_ancestor_weights 
  //    = *entity_ancestor_weights_[entity_from]; 
  //const map<int, float>& entity_to_ancestor_weights 
  //    = *entity_ancestor_weights_[entity_to]; 
  //queue<int> unprocessed_nodes;
  //set<int> processed_nodes;
  //// expand from common ancestors
  //set<int>::const_iterator set_it_ = common_ancestors.begin();
  //for (; set_it_ != common_ancestors.end(); ++set_it_) {
  //  unprocessed_nodes.push(*set_it_);
  //}
  //int cur_idx;
  //float weight_sum = 0;
  //float node_weight_sum;
  //int node_weight_cnt;
  //while (!unprocessed_nodes.empty()) {
  //  cur_idx = unprocessed_nodes.front();
  //  unprocessed_nodes.pop();
  //  if (processed_nodes.find(cur_idx) != processed_nodes.end()) {
  //    continue;
  //  } 
  //  processed_nodes.insert(cur_idx);
  //  node_weight_sum = 0;
  //  node_weight_cnt = 0;
  //  map<int, float>::const_iterator map_cit_ 
  //      = entity_from_ancestor_weights.find(cur_idx); 
  //  if (map_cit_ != entity_from_ancestor_weights.end()) {
  //    node_weight_sum += map_cit_->second;
  //    ++node_weight_cnt;
  //  }
  //  map_cit_ = entity_to_ancestor_weights.find(cur_idx); 
  //  if (map_cit_ != entity_to_ancestor_weights.end()) {
  //    node_weight_sum += map_cit_->second;
  //    ++node_weight_cnt;
  //  }
  //  if (node_weight_cnt > 0) { // ancestor of entity_from or entity_to
  //    weight_sum += node_weight_sum;
  //    path->AddCategoryNode(nodes_[cur_idx]->id(), node_weight_sum);
  //    // expand children
  //    const vector<int>& cur_children = nodes_[cur_idx]->child_idx();
  //    for (int c_idx_i = 0; c_idx_i < cur_children.size(); ++c_idx_i) {
  //      unprocessed_nodes.push(cur_children[c_idx_i]);
  //    }
  //  }
  //} // end of expansion

#ifdef DEBUG
  CHECK_GT(weight_sum, 0);
  //CHECK_GT(ca_weight_sum, 0);
  //CHECK_GT(max_ca_weight, 0);
  CHECK_EQ(ca_weights.size(), common_ancestors.size());
#endif
  //LOG(INFO) << ca_weight_sum << " / " << num_ca << " / " << weight_sum;
  //path->ScaleCategoryWeights(1.0 / (weight_sum * /* sqrt(num_ca) */ ca_weight_sum)); 
  float max_ca_weight = 0;
  map<int, float>::const_iterator it = ca_weights.begin();
  for (; it != ca_weights.end(); ++it) {
    max_ca_weight = max(it->second, max_ca_weight);
  }

  path->ScaleCategoryWeights(1.0 / weight_sum, 1.0 / max_ca_weight); 

  //LOG(INFO) << "expand common ancestor: "
  //    << ((double)(clock() - t_start) / CLOCKS_PER_SEC);
  //expand_time += ((double)(clock() - t_start) / CLOCKS_PER_SEC);

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

/**
 * ===============================
 * NOTE: bug
 *   C_1 -> entity_from
 *   C_2 -> entity_from
 *   C_3 -> C_4
 *   C_4 -> entity_from
 *   C_4 -> entity_to
 *
 *   then cannot identify C_1
 * ================================
 *
 * @param [IN] entity_from
 * @param [IN] entity_to
 * @param [OUT] entity_from_ancestor_weights: 
 *     idx => #path to entity_from
 * @param [OUT] entity_to_ancestor_weights:
 *     idx => #path to entity_to
 * @param [OUT] common_ancestors:
 *     idx of common ancestors that have disjoint paths to entity_from 
 *     and entity_to;
 *
 * Note: the designed alg. is provably right. Consider as a contribution
 *       in the paper?
 */
//void Hierarchy::FindCommonAncestors(const int entity_from, const int entity_to,
//    map<int, int>& entity_from_ancestor_weights, 
//    map<int, int>& entity_to_ancestor_weights,
//    set<int>& common_ancestors) {
//  /// get the ancestor nodes of entity_from 
//  queue<int> unprocessed_nodes;
//  unprocessed_nodes.push(entity_from);
//  int cur_node_idx;
//  while (!unprocessed_nodes.empty()) {
//    cur_node_idx = unprocessed_nodes.pop();
//    ++entity_from_ancestor_weights[cur_node_idx];
//    const vector<int>& cur_parents = nodes_[cur_node_idx]->parent_idx();
//    for (int p_idx_i = 0; p_idx_i < cur_parents.size(); ++p_idx_i) {
//      unprocessed_nodes.push(cur_parents[i]);
//    }
//  } // entity_from is included in entity_from_ancestor_weights
//
//  /// get the ancestor nodes of entity_to and common ancestors 
//  int cur_parent_idx;
//  // nodes in unprocessed_nodes are either non-common ancestors, or
//  // common ancestors that have a disjoint path to entity_from/to
//  unprocessed_nodes.push(entity_to);
//  while (!unprocessed_nodes.empty()) {
//    cur_node_idx = unprocessed_nodes.pop();
//    ++entity_to_ancestor_weights[cur_node_idx];
//    const vector<int>& cur_parents = nodes_[cur_node_idx]->parent_idx();
//    for (int p_idx_i = 0; p_idx_i < cur_parents.size(); ++p_idx_i) {
//      cur_parent_idx = cur_parents[p_idx_i];
//      // common ancestor: the node is an ancestor of entity_from AND
//      // has a child (!= cur_node) that is also an ancestor of entity_from (or 
//      // entity_from itself)
//      if (entity_from_ancestor_weights.find(cur_parent_idx) 
//          != entity_from_ancestor_weights.end() &&
//          common_ancestors.find(cur_parent_idx) // make sure no duplicate
//          == common_ancestors.end()) {
//        const vector<int>* cur_parent_children 
//            = &nodes_[cur_parent_idx]->child_idx();
//        int c_idx_i = 0;
//        for (; c_idx_i < cur_parant_children->size(); ++c_idx_i) {
//          int c_idx = cur_parent_children->at(idx);
//          if (c_idx != cur_node_idx && entity_from_ancestor_weights.find(c_idx) 
//              != entity_from_ancestor_weights.end()) {
//            break;
//          }
//        }
//        if (c_idx_i < cur_parent_children->size()) {
//          common_ancestors.insert(cur_parent_idx);
//          unprocessed_nodes.push(cur_parent_idx);
//        } // else, do not traverse cur_parent's parents
//      } else {
//        unprocessed_nodes.push(cur_parent_idx);
//      }
// 
//    } // end of traverse all parent nodes
//  }
//
//  /// de-allocate memory
//  std::queue<int>().swap(unprocessed_nodes); 
//}

}  // namespace entity
