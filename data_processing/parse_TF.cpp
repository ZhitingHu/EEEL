#include <cstdlib>
#include <iostream>
#include <string>
#include <iterator>
#include <fstream>
#include <vector>
#include <sstream>
#include <map>
#include <algorithm>
#define Freq_Thd 0

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

   // Open files
   string dataset_path = "/home/zhitingh/ml_proj/EEEL/data/whole_parsed/";
   string pair_filename = "pair.txt";
   string pair_TF_filename = "pair_term_freq.txt";
  
   ifstream pair_file((dataset_path + "/" + pair_filename).c_str());

   if (!pair_file.is_open())
      cout << "fail to open:" << dataset_path + "/" + pair_filename << "\n";
   
   // Parse pair File
   while (getline(pair_file, line)){
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
      pair_TF_sorted_file << entity_freq_vec[i].first << "\t" << entity_freq_vec[i].second << endl;
   }

   // Close files
   pair_file.close();
   pair_TF_sorted_file.close();

   cout << "Done." << endl;
   return 0;
}
