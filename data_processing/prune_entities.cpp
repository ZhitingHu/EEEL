#include <cstdlib>
#include <iostream>
#include <string>
#include <iterator>
#include <fstream>
#include <vector>
#include <sstream>
#include <map>

using namespace std;


inline bool prune(const string& str) {
  if (str.compare(0, 9,  "Category:") == 0 || 
      str.compare(0, 10, "Wikipedia:") == 0 ||
      str.compare(0, 5,  "File:") == 0 ||
      str.compare(0, 9,  "Template:") == 0 ||
      str.compare(0, 5,  "Help:") == 0 ||
      str.compare(0, 5,  "Draft:") == 0 ||
      str.compare("Wikipedia") == 0 ||
      str.compare("Image") == 0 ||
      str.compare(0, 8, "List_of_") == 0 ||
      str.length() == 1 ||
      str.find_first_not_of("0123456789") == std::string::npos) {
    return true;
  } else {
    return false;
  }
}

int main(){
   // Parsing
   string line;
   vector<int> effective;
   vector<int> removed_entity;
   int effective_idx = 0;
   int removed_entity_idx = 0;
   int old_entity_idx = 0;
   int num_entity_ = 0;
   int new_entity_idx = 0;
   // old_entity_idx => new_entity_idx
   map<int, int> idx_map;
   
   // Open files
   string dataset_path = "/home/zhitingh/ml_proj/EEEL/data/apple/origin";
   string parsed_dataset_path = "/home/zhitingh/ml_proj/EEEL/data/apple/pruned_admin_and_meaningless_entities";

   string effective_entity_ids_filename = "effective_entity_ids.txt";
   //string category_filename = "categories.txt";
   string entity_filename = "entity.txt";
   //string entity_to_ancestor_filename = "entity2ancestor.txt";
   string entity_to_category_filename = "entity2category.txt";
   //string hierarchy_filename = "hierarchy.txt";
   //string hierarchy_id_filename = "hierarchy_id.txt";
   string pair_filename = "pair.txt";
   //string level_filename = "level.txt";
  			
   // Parse Effective Entities
   ifstream effective_entity_ids_file((dataset_path + "/" + effective_entity_ids_filename).c_str()); 
   if (!effective_entity_ids_file.is_open())
      cout << "fail to open:" << dataset_path + "/" + effective_entity_ids_filename << "\n"; 
   while (getline(effective_entity_ids_file, line)){
      stringstream ss(line);
      int temp_id;
      ss >> temp_id;
      effective.push_back(temp_id);
   }
   cout << "number of effective ids: " << effective.size() << endl;
   effective_entity_ids_file.close();
   
   ifstream entity_file((dataset_path + "/" + entity_filename).c_str());
   ifstream entity_category_file((dataset_path + "/" + entity_to_category_filename).c_str());
   ifstream pair_file((dataset_path + "/" + pair_filename).c_str());

   ofstream parsed_entity_file((parsed_dataset_path + "/" + entity_filename).c_str());
   ofstream parsed_entity_category_file((parsed_dataset_path + "/" + entity_to_category_filename).c_str());
   ofstream parsed_pair_file((parsed_dataset_path + "/" + pair_filename).c_str());

   if (!entity_file.is_open())
      cout << "fail to open:" << dataset_path + "/" + entity_filename << "\n";
   if (!entity_category_file.is_open())
      cout << "fail to open:" << dataset_path + "/" + entity_to_category_filename << "\n";
   if (!pair_file.is_open())
      cout << "fail to open:" << dataset_path + "/" + pair_filename << "\n";

   if (!parsed_entity_file.is_open())
      cout << "fail to open:" << parsed_dataset_path + "/" + entity_filename << "\n";
   if (!parsed_entity_category_file.is_open())
      cout << "fail to open:" << parsed_dataset_path + "/" + entity_to_category_filename << "\n";
   if (!parsed_pair_file.is_open())
      cout << "fail to open:" << parsed_dataset_path + "/" + pair_filename << "\n";

  
   effective_idx = 0;
   new_entity_idx = 0;
   old_entity_idx = 0;
   // Parse entity file
   while (getline(entity_file, line)){
      if (prune(line)) {
         if (effective[effective_idx] == old_entity_idx) {
            effective_idx++;
         }
         // insert to mapping (invalid)
         idx_map.insert(pair<int, int>(old_entity_idx, -1));
         removed_entity.push_back(old_entity_idx);
      }
      else{
         if (effective[effective_idx] == old_entity_idx){
            // effective and pre: OK
            parsed_entity_file << line << endl;
            idx_map.insert(pair<int, int>(old_entity_idx, new_entity_idx));
            new_entity_idx++;
            effective_idx++;
         } else{
            // insert to mapping (invalid)
            idx_map.insert(pair<int, int>(old_entity_idx, -1));
            removed_entity.push_back(old_entity_idx);
         }
      }
      old_entity_idx++;
   } 

   cout << "number of entity: " << old_entity_idx << endl;
   cout << "number of entity to remove: " << removed_entity.size() << endl;
   cout << "number of entity remained: " << (old_entity_idx - removed_entity.size()) << endl;
   cout << "mapping table size:" << idx_map.size() << endl;
   cout << "Last two:" << idx_map[idx_map.size()-2] << endl; 
   cout << "Last one:" << idx_map[idx_map.size()-1] << endl;

   // entity-category file
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

   //// pair File
   while (getline(pair_file, line)){
     istringstream iss(line);
     vector<int> tokens( (std::istream_iterator<int>(iss)), (std::istream_iterator<int>()) );
     
     if (tokens.size() != 3){
       cout << "Err: Pair format should with len 3" << endl;
       break;
     }

     if (idx_map[tokens[0]] != -1 && idx_map[tokens[1]] != -1) {
       parsed_pair_file << idx_map[tokens[0]] << "\t" << idx_map[tokens[1]] << "\t" << tokens[2] << endl;
     }
   }

   entity_file.close();
   entity_category_file.close();
   pair_file.close();

   parsed_entity_file.close();
   parsed_entity_category_file.close();
   parsed_pair_file.close();
   
   cout << "parsing done!" << endl;
   return 0;
}
