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

int main(){
   string line;
   vector<string> lines;
 
   string path = "/home/zhitingh/ml_proj/EEEL/data/whole/";
   string pair_filename = "pair.txt";
   string shuffled_pair_filename = "shuffled_pair.txt";

   ifstream pair_file((path + "/" + pair_filename).c_str());
   ofstream shuffled_pair_file((path + "/" + shuffled_pair_filename).c_str());

   if (!pair_file.is_open())
      cout << "fail to open:" << path + "/" + pair_filename << "\n";
   if (!shuffled_pair_file.is_open())
      cout << "fail to open:" << path + "/" + shuffled_pair_filename << "\n";

   cout << "Reading" << endl; 
   while (getline(pair_file, line)){
     lines.push_back(line);
   }

   cout << "Shuffling" << endl; 
   random_shuffle(lines.begin(), lines.end());

   cout << "Writing" << endl; 
   for (int i = 0; i < lines.size(); ++i) {
     shuffled_pair_file << lines[i] << endl;
   }   
   
   // Close files
   pair_file.close();
   shuffled_pair_file.close();

   cout << "Done." << endl;

   return 0;
}
