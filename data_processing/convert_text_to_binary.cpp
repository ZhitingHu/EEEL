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

   string path = "/home/zhitingh/ml_proj/EEEL/data/tech_parsed/";
   string txt_filename = "entity2ancestor.txt";
   string binary_filename = "entity2ancestor.binary";

   ifstream txt_file((path + "/" + txt_filename).c_str());
   ofstream binary_file((path + "/" + binary_filename).c_str(), ios::out | ios::binary);

   if (!txt_file.is_open())
      cout << "fail to open:" << path + "/" + txt_filename << "\n";
   if (!binary_file.is_open())
      cout << "fail to open:" << path + "/" + binary_filename << "\n";

   while (getline(txt_file, line)){
     

   }

   cout << "Shuffling" << endl; 
   random_shuffle(lines.begin(), lines.end());

   cout << "Writing" << endl; 
   for (int i = 0; i < lines.size(); ++i) {
     binary_file << lines[i] << endl;
   }   
   
   // Close files
   txt_file.close();
   binary_file.close();

   cout << "Done." << endl;

   return 0;
}
