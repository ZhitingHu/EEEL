// Author: Dai Wei (wdai@cs.cmu.edu)
// Date: 2014.07.11

#include "string_util.hpp"

namespace util {

void split(const std::string &s, char delim,
    std::vector<std::string>* elems) {
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, delim)) {
    elems->push_back(item);
  }
}

std::vector<std::string> split(const std::string& s, char delim) {
  std::vector<std::string> elems;
  split(s, delim, &elems);
  return elems;
}

}   // namespace util
