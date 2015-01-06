#include <cstdlib>
#include <iostream>
#include <cstring>
#include <iterator>
#include <fstream>
#include <vector>
#include <sstream>
#include <map>
#include <set>
#include <algorithm>

using namespace std;

#define Freq_Thd 1

struct sort_val {
   bool operator()(const std::pair<int,int> &left, const std::pair<int,int> &right) {
       return left.second > right.second;
    }
};

int main(){
   string line;
   // old entity id => freq
   //map<int, int> entity_freq;
   // old entity id => name
   map<int, string> entity_id_name; 

   // old entity id => new entity id
   map<int, int> old_entity_id_to_new_id;

   // by mention-entity-dictionary
   set<string> mentioned_entities;

   // Open files
   string dataset_path = "/home/zhitingh/ml_proj/EEEL/data/apple/pruned_admin_and_meaningless_entities/";

   string entity_filename = "entity.txt";
   string entity_to_category_filename = "entity2category.txt";
   string pair_filename = "pair.txt";
   string entity_freq_filename = "entity_freq_sorted.txt";
   string mention_to_entity_filename = "mention_entity_map";
  
   ifstream entity_file((dataset_path + "/" + entity_filename).c_str());
   ifstream entity_category_file((dataset_path + "/" + entity_to_category_filename).c_str());
   ifstream pair_file((dataset_path + "/" + pair_filename).c_str());
   ifstream entity_freq_file((dataset_path + "/" + entity_freq_filename).c_str());
   ifstream mention_to_entity_file("/home/zhitingh/ml_proj/EEEL/data/entity_linking/mention_entity_map");

   ofstream remained_entity_file((dataset_path + "/entity_remained.txt").c_str());
   ofstream new_entity_category_file((dataset_path + "/new_" + entity_to_category_filename).c_str());
   ofstream mentioned_entity_with_low_freq_file((dataset_path + "/entity_in_dictionary_but_low_freq_in_pair.txt").c_str());
   ofstream new_pair_file((dataset_path + "/new_pair.txt").c_str());

   if (!entity_file.is_open())
      cout << "fail to open:" << dataset_path + "/" + pair_filename << "\n";
   if (!pair_file.is_open())
      cout << "fail to open:" << dataset_path + "/" + pair_filename << "\n";

   // read mention entity file  
   int cnt = 0; 
   while (getline(mention_to_entity_file, line)) {
     char* dup = strdup(line.c_str());
     char* parts = strtok(dup, "\t");
     parts = strtok(NULL, "\t");
     mentioned_entities.insert(parts);

     cnt++;
     if (cnt % 200000 == 0) {
       cout << "." << flush;
     }
   }
   cout << endl;
   cout << "#lines of mention-entity-dictionary: " << cnt << endl;

   // read entity file
   int old_entity_id = 0;
   while (getline(entity_file, line)) {
     entity_id_name[old_entity_id] = line;
     old_entity_id++;
   }
   cout << "Num of entities: " << old_entity_id << " = " << entity_id_name.size() << endl;
 
   // read entity freq file
   int freq;
   int new_entity_id = 0;
   int num_of_mentioned_entity_with_low_freq = 0;
   cnt = 0;
   string entity_name;
   while (entity_freq_file >> old_entity_id >> freq >> entity_name) {
     if (freq > Freq_Thd ||
         mentioned_entities.find(entity_name) != mentioned_entities.end()) {
       remained_entity_file << entity_name << endl;
       old_entity_id_to_new_id[old_entity_id] = new_entity_id;
       new_entity_id++;
       if (freq <= Freq_Thd) {
         mentioned_entity_with_low_freq_file << entity_name << "\t" << freq << endl;
         num_of_mentioned_entity_with_low_freq++;
       }
     } else {
       cnt++;
     }
   }
   cout << "Removed entities " << cnt << endl;

   // entity category file
   while (getline(entity_category_file, line)){
     istringstream iss(line);
     vector<int> tokens( (std::istream_iterator<int>(iss)), (std::istream_iterator<int>()) );
    
     old_entity_id = tokens[0];
     if (old_entity_id_to_new_id.find(old_entity_id) == old_entity_id_to_new_id.end()) {
       continue;
     }
     new_entity_id = old_entity_id_to_new_id[old_entity_id];
     new_entity_category_file << new_entity_id << "\t";
     for (unsigned int token_idx = 1; token_idx < tokens.size(); ++token_idx){
        new_entity_category_file << tokens[token_idx] << "\t";
     }
     new_entity_category_file << endl;
   }

   // pair file
   cnt = 0;
   map<int, int>::const_iterator map_it_1, map_it_2;
   while (getline(pair_file, line)) {
     istringstream iss(line);
     vector<int> tokens((std::istream_iterator<int>(iss)), (std::istream_iterator<int>()) );
     
     if (tokens.size() != 3){
        cout << "Err: Pair format should with len 3" << endl;
        break;
     }
     
     map_it_1 = old_entity_id_to_new_id.find(tokens[0]);
     map_it_2 = old_entity_id_to_new_id.find(tokens[1]);
     if (map_it_1 != old_entity_id_to_new_id.end() 
         && map_it_2 != old_entity_id_to_new_id.end()) {
       new_pair_file << map_it_1->second << "\t" << map_it_2->second << "\t" << tokens[2] << endl;
       cnt++;
     }
   }
   cout << "Num of pairs " << cnt << endl;

   // close files
   entity_file.close();
   new_entity_category_file.close();
   pair_file.close();
   entity_freq_file.close();
   mention_to_entity_file.close();
   remained_entity_file.close();
   mentioned_entity_with_low_freq_file.close();
   new_pair_file.close();

   cout << "Done." << endl;
   return 0;
}
