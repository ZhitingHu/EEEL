#include <cstdlib>
#include <iostream>
#include <cstring>
#include <iterator>
#include <fstream>
#include <vector>
#include <sstream>
#include <map>
#include <set>

using namespace std;

int main(){
   string line;
   map<string, int> entity_name_id;
   set<string> mentioned_entities;
 
   string path = "/home/zhitingh/ml_proj/EEEL/data/whole_parsed/";
   string entity_filename = "pair.txt";
   string mention_to_entity_filename = "mention_entity_map";
   string mentioned_entity_id_filename = "mentioned_entity_id";
   string removed_entity_id_filename = "removed_entity_id";

   ifstream entity_file((path + "/" + entity_filename).c_str());
   ifstream mention_to_entity_file((path + "/" + mention_to_entity_filename).c_str());

   ofstream mentioned_entity_id_file((path + "/" + mentioned_entity_id_filename).c_str());
   ofstream removed_entity_id_file((path + "/" + removed_entity_id_filename).c_str());

   if (!entity_file.is_open())
      cout << "fail to open:" << path + "/" + entity_filename << "\n";
   if (!mention_to_entity_file.is_open())
      cout << "fail to open:" << path + "/" + mention_to_entity_filename << "\n";
   if (!mentioned_entity_id_file.is_open())
      cout << "fail to open:" << path + "/" + mentioned_entity_id_filename << "\n";
   if (!removed_entity_id_file.is_open())
      cout << "fail to open:" << path + "/" + removed_entity_id_filename << "\n";
 
   int cnt = 0; 
   while (getline(mention_to_entity_file, line)){
     char* dup = strdup(line.c_str());
     char* parts = strtok(dup, "\t");
     parts = strtok(NULL, "\t");
     mentioned_entities.insert(parts);

     //cout << parts << endl;
     //if (cnt == 100) {
     //  return 0;
     //}

     cnt++;
     if (cnt % 200000 == 0) {
       cout << "." << flush;
     }
   }

   int entity_id = 0; 
   int effective_count = 0;
   cnt = 0;
   while (getline(entity_file, line)) {
     if (mentioned_entities.find(line) != mentioned_entities.end()) {
       //entity_name_id[line] = entity_id;
       mentioned_entity_id_file << line << "\t" << entity_id << endl;
       effective_count++;
     } else {
       removed_entity_id_file << line << "\t" << entity_id << endl;
     }
     ++entity_id; 

     cnt++;
     if (cnt % 200000 == 0) {
       cout << "." << flush;
     }
   } 

   cout << "number of entity: " << entity_id << endl;
   cout << "number of mentioned entity: " << effective_count << endl;

   // Close files
   entity_file.close();
   mention_to_entity_file.close();
   mentioned_entity_id_file.close();
   removed_entity_id_file.close();

   cout << "Done." << endl;

   return 0;
}
