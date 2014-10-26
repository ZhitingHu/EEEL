// Author: Dai Wei (wdai@cs.cmu.edu)
// Date: 2014.07.11

#pragma once

#include <sstream>
#include <string>
#include <vector>

namespace util {

// Split string s by delimintor char delim (output in elems).
void split(const std::string &s, char delim,
    std::vector<std::string>* elems);

// Split string s by delimintor char delim.
std::vector<std::string> split(const std::string& s, char delim);

}  // namespace util
