#include <cstdlib>
#include <iostream>
#include <string>
#include <iterator>
#include <fstream>
#include <vector>
#include <sstream>
#include <map>

using namespace std;

int main(){
  
   // Parsing
   string line;
   vector<int> effective;
   vector<int> target;
   int effective_idx = 0;
   int target_idx = 0;
   int idx = 0;
   int num_entity_ = 0;
   int entity_idx = 0;
   map<int, int> idx_map;
   
   // Open files
   string dataset_path = "whole";
   string parsed_dataset_path = "test";

   string effective_entity_ids_filename = "effective_entity_ids.txt";
   //string category_filename = "categories.txt";
   string org_entity_filename = "entity.txt";
   string entity_filename = "entity.txt";
   //string entity_to_ancestor_filename = "entity2ancestor.txt";
   string entity_to_category_filename = "entity2category.txt";
   //string hierarchy_filename = "hierarchy.txt";
   //string hierarchy_id_filename = "hierarchy_id.txt";
   //string pair_filename = "pair.txt";
   //string level_filename = "level.txt";
  			
   
   // Parse Effective Entities
   ifstream effective_entity_ids_file((dataset_path + "/" + effective_entity_ids_filename).c_str()); 
   if (!effective_entity_ids_file.is_open())
      cout << "fail to open:" << dataset_path + "/" + effective_entity_ids_filename << "\n"; 
   // 1.1 Parse Effective Entity File
   idx = 0;
   while (getline(effective_entity_ids_file, line)){
      //effective.push_back(stoi(line));
      stringstream ss(line);
      int temp_id;
      ss >> temp_id;
      effective.push_back(temp_id);
      idx++;
   }
   cout << "number of effective ids: " << idx << endl;
   effective_entity_ids_file.close();
  
   
   // to parse 
   //ifstream category_file((dataset_path + "/" + category_filename).c_str());
   ifstream entity_file((dataset_path + "/" + org_entity_filename).c_str());
   //ifstream entity_ancestor_file((dataset_path + "/" + entity_to_ancestor_filename).c_str());
   ifstream entity_category_file((dataset_path + "/" + entity_to_category_filename).c_str());
   //ifstream hierarchy_file((dataset_path + "/" + hierarchy_filename).c_str());
   //ifstream hierarchy_id_file((dataset_path + "/" + hierarchy_id_filename).c_str());
   //ifstream pair_file((dataset_path + "/" + pair_filename).c_str());
   //ifstream level_file((dataset_path + "/" + level_filename).c_str());

   // parsed
   //ofstream parsed_category_file((parsed_dataset_path + "/" + category_filename).c_str());
   ofstream parsed_entity_file((parsed_dataset_path + "/" + entity_filename).c_str());
   //ofstream parsed_entity_ancestor_file((parsed_dataset_path + "/" + entity_to_ancestor_filename).c_str());
   ofstream parsed_entity_category_file((parsed_dataset_path + "/" + entity_to_category_filename).c_str());
   //ofstream parsed_hierarchy_file((parsed_dataset_path + "/" + hierarchy_filename).c_str());
   //ofstream parsed_hierarchy_id_file((parsed_dataset_path + "/" + hierarchy_id_filename).c_str());
   //ofstream parsed_pair_file((parsed_dataset_path + "/" + pair_filename).c_str());
   //ofstream parsed_level_file((parsed_dataset_path + "/" + level_filename).c_str());



   //if (!category_file.is_open())
   //   cout << "fail to open:" << dataset_path + "/" + category_filename << "\n";
   if (!entity_file.is_open())
      cout << "fail to open:" << dataset_path + "/" + org_entity_filename << "\n";
   //if (!entity_ancestor_file.is_open())
   //   cout << "fail to open:" << dataset_path + "/" + entity_to_ancestor_filename << "\n";
   if (!entity_category_file.is_open())
      cout << "fail to open:" << dataset_path + "/" + entity_to_category_filename << "\n";
   //if (!hierarchy_file.is_open())
   //   cout << "fail to open:" << dataset_path + "/" + hierarchy_filename << "\n";
   //if (!hierarchy_id_file.is_open())
   //   cout << "fail to open:" << dataset_path + "/" + hierarchy_id_filename << "\n";
   //if (!pair_file.is_open())
   //   cout << "fail to open:" << dataset_path + "/" + pair_filename << "\n";
   //if (!level_file.is_open())
   //   cout << "fail to open:" << dataset_path + "/" + level_filename << "\n";

   //if (!parsed_category_file.is_open())
   //   cout << "fail to open:" << parsed_dataset_path + "/" + category_filename << "\n";
   if (!parsed_entity_file.is_open())
      cout << "fail to open:" << parsed_dataset_path + "/" + entity_filename << "\n";
   //if (!parsed_entity_ancestor_file.is_open())
   //   cout << "fail to open:" << parsed_dataset_path + "/" + entity_to_ancestor_filename << "\n";
   if (!parsed_entity_category_file.is_open())
      cout << "fail to open:" << parsed_dataset_path + "/" + entity_to_category_filename << "\n";
   //if (!parsed_hierarchy_file.is_open())
   //   cout << "fail to open:" << parsed_dataset_path + "/" + hierarchy_filename << "\n";
   //if (!parsed_hierarchy_id_file.is_open())
   //   cout << "fail to open:" << parsed_dataset_path + "/" + hierarchy_id_filename << "\n";
   //if (!parsed_pair_file.is_open())
   //   cout << "fail to open:" << parsed_dataset_path + "/" + pair_filename << "\n";
   //if (!parsed_level_file.is_open())
   //  cout << "fail to open:" << parsed_dataset_path + "/" + level_filename << "\n";
  
   //idx = 0; 
   //// 1. Parse Category File
   //while (getline(category_file, line)){
   //   parsed_category_file << line << endl;
   //   idx++;
   //}
   //cout << "number of category: " << idx << endl;
   
   
   effective_idx = 0;
   entity_idx = 0;
   idx = 0;
   // 2. Parse Entity File
   while (getline(entity_file, line)){
      if (line.compare(0, 9,  "Category:") == 0 || 
          line.compare(0, 10, "Wikipedia:") == 0 ||
          line.compare(0, 5,  "File:") == 0 ||
          line.compare(0, 9,  "Template:") == 0 ||
          line.compare(0, 5,  "Help:") == 0 ||
          line.compare(0, 5,  "Draft:") == 0
          ){
         if (effective[effective_idx] == idx)
            effective_idx++;
         // insert to mapping (invalid)
         idx_map.insert(pair<int, int>(idx, -1));
         target.push_back(idx);
      }
      else{
         if (effective[effective_idx] == idx){
            // effective and pre: OK
            parsed_entity_file << line << endl;
            idx_map.insert(pair<int, int>(idx, entity_idx));
            entity_idx++;
            effective_idx++;
         } else{
            // insert to mapping (invalid)
            idx_map.insert(pair<int, int>(idx, -1));
            target.push_back(idx);
         }
      }
      idx++;
   } 

   cout << "number of entity: " << idx << endl;
   cout << "number of target to remove (Wikipedia/Help/Template/Draft/File/Category:entity): " << target.size() << endl;
   cout << "mapping table size:" << idx_map.size() << endl;
   cout << "Last two:" << idx_map[idx_map.size()-2] << endl; 
   cout << "Last one:" << idx_map[idx_map.size()-1] << endl;

   // 3. Parse Entity-Category file
   //target_idx = 0;
   //idx = 0;
   //entity_idx = 0;
   //while (getline(entity_category_file, line)){
   //   istringstream iss(line);
   //   vector<int> tokens( (std::istream_iterator<int>(iss)), (std::istream_iterator<int>()) );

   //   if (idx <= target[target.size() - 1]){
   //      if (target[target_idx] == idx){
   //         target_idx++;
   //      }
   //      else{
   //         //parsed_entity_category_file << entity_idx << "\t";
   //         parsed_entity_category_file << idx_map[idx] << "\t";
   //         for (unsigned int token_idx = 1; token_idx < tokens.size(); ++token_idx){
   //            parsed_entity_category_file << tokens[token_idx] << "\t";
   //         }
   //         parsed_entity_category_file << endl;
   //         entity_idx++;
   //      }
   //   }
   //   else{
   //      //parsed_entity_category_file << entity_idx << "\t";
   //      parsed_entity_category_file << idx_map[idx] << "\t";
   //      for (unsigned int token_idx = 1; token_idx < tokens.size(); ++token_idx){
   //         parsed_entity_category_file << tokens[token_idx] << "\t";
   //      }
   //      parsed_entity_category_file << endl;
   //      entity_idx++;
   //   }
   //   idx++;
   //}

   target_idx = 0;
   idx = 0;
   entity_idx = 0;
   while (getline(entity_category_file, line)){
     istringstream iss(line);
     vector<int> tokens( (std::istream_iterator<int>(iss)), (std::istream_iterator<int>()) );
    
     int old_entity_id = tokens[0];
     int new_entity_id = idx_map[old_entity_id];
     if (new_entity_id != -1) {
       parsed_entity_category_file << new_entity_id << "\t";
       for (unsigned int token_idx = 1; token_idx < tokens.size(); ++token_idx){
          parsed_entity_category_file << tokens[token_idx] << "\t";
       }
       parsed_entity_category_file << endl;
     }
   }

   // 4. Parse hierarchy_id (category)
   //while (getline(hierarchy_id_file, line)){
   //   parsed_hierarchy_id_file << line << endl;
   //}
   
   // 5. Parse entity ancestor File
   //target_idx = 0;
   //idx = 0;
   //entity_idx = 0;
   //while (getline(entity_ancestor_file, line)){
   //   istringstream iss(line);
   //   vector<string> tokens( (std::istream_iterator<string>(iss)), (std::istream_iterator<string>()) );

   //   if (idx <= target[target.size() - 1]){
   //      if (target[target_idx] == idx){
   //         target_idx++;
   //      }
   //      else{
   //         parsed_entity_ancestor_file << idx_map[idx] << "\t";
   //         for (unsigned int token_idx = 1; token_idx < tokens.size(); ++token_idx){
   //            parsed_entity_ancestor_file << tokens[token_idx] << "\t";
   //         }
   //         parsed_entity_ancestor_file << endl;
   //         entity_idx++;
   //      }
   //   }
   //   else{
   //      parsed_entity_ancestor_file << idx_map[idx] << "\t";
   //      for (unsigned int token_idx = 1; token_idx < tokens.size(); ++token_idx){
   //         parsed_entity_ancestor_file << tokens[token_idx] << "\t";
   //      }
   //      parsed_entity_ancestor_file << endl;
   //      entity_idx++;
   //   }
   //   idx++;
   //}

  
   //ofstream pair_debug_file((parsed_dataset_path + "/" + "debug_pair.txt").c_str());

   // 6. Parse pair File
   //while (getline(pair_file, line)){
   //   istringstream iss(line);
   //   vector<int> tokens( (std::istream_iterator<int>(iss)), (std::istream_iterator<int>()) );
   //   
   //   if (tokens.size() != 3){
   //      cout << "Err: Pair format should with len 3" << endl;
   //      break;
   //   }

   //   if (idx_map[tokens[0]] != -1 && idx_map[tokens[1]] != -1)
   //      parsed_pair_file << idx_map[tokens[0]] << "\t" << idx_map[tokens[1]] << "\t" << tokens[2] << endl;
   //   
   //   pair_debug_file << idx_map[tokens[0]] << "\t" << idx_map[tokens[1]] << "\t" << tokens[2] << endl;

   //}


   
   // 7. Parse hierarchy (category)
   //while (getline(hierarchy_file, line)){
   //   parsed_hierarchy_file << line << endl;
   //}
   
   // 8. Parse level file (category)
   //while (getline(level_file, line)){
   //   parsed_level_file << line << endl;
   //}
   
   // Close files
   //category_file.close();
   entity_file.close();
   //entity_ancestor_file.close();
   entity_category_file.close();
   //hierarchy_file.close();
   //hierarchy_id_file.close();
   //pair_file.close();
   //level_file.close();
   

   cout << "parsing done!" << endl;
   //system("pause");
   return 0;
}
