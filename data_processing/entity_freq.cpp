#include <cstdlib>
#include <iostream>
#include <string>
#include <iterator>
#include <fstream>
#include <vector>
#include <sstream>
#include <map>
#include <algorithm>

using namespace std;

struct sort_val {
   bool operator()(const std::pair<int,int> &left, const std::pair<int,int> &right) {
       return left.second > right.second;
    }
};

int main(){
   // Parsing
   string line;
   map<int, int> entity_freq;
   map<int, string> entity_id_name; 

   // Open files
   string dataset_path = "/home/zhitingh/ml_proj/EEEL/data/whole_new/pruned_admin_and_meaningless_entities/";
   string entity_filename = "entity.txt";
   string pair_filename = "pair.txt";
   string pair_TF_filename = "pair_term_freq.txt";
  
   ifstream entity_file((dataset_path + "/" + entity_filename).c_str());
   ifstream pair_file((dataset_path + "/" + pair_filename).c_str());

   if (!entity_file.is_open())
      cout << "fail to open:" << dataset_path + "/" + pair_filename << "\n";
   if (!pair_file.is_open())
      cout << "fail to open:" << dataset_path + "/" + pair_filename << "\n";
  
   // Read entity file
   int entity_id = 0;
   while (getline(entity_file, line)) {
     entity_id_name[entity_id] = line;
     entity_id++;
   }
   cout << "Num of entities: " << entity_id << " = " << entity_id_name.size() << endl;
 
   // Parse pair File
   while (getline(pair_file, line)) {
     istringstream iss(line);
     vector<int> tokens((std::istream_iterator<int>(iss)), (std::istream_iterator<int>()) );
     
     if (tokens.size() != 3){
        cout << "Err: Pair format should with len 3" << endl;
        break;
     }
     entity_freq[tokens[0]]++;
     entity_freq[tokens[1]]++;
   }

   vector<pair<int, int> > entity_freq_vec;
   map<int, int>::const_iterator it = entity_freq.begin();
   for (; it != entity_freq.end(); ++it) {
     entity_freq_vec.push_back(pair<int, int>(it->first, it->second));
   }

   sort(entity_freq_vec.begin(), entity_freq_vec.end(), sort_val());

   ofstream pair_TF_sorted_file((dataset_path + "/" + "entity_freq_sorted.txt").c_str());
   for (int i = 0; i < entity_freq_vec.size(); ++i){
     int cur_entity_id = entity_freq_vec[i].first;
     if (entity_id_name.find(cur_entity_id) == entity_id_name.end()) {
       cerr << "Err: entity id not found in entity_id_name " << cur_entity_id << endl;
       exit(0);
     }
     pair_TF_sorted_file << cur_entity_id << "\t" << entity_freq_vec[i].second 
         << "\t" << entity_id_name[cur_entity_id] << endl;
   }

   // close files
   entity_file.close();
   pair_file.close();
   pair_TF_sorted_file.close();

   cout << "Done." << endl;
   return 0;
}
