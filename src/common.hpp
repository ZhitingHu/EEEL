#pragma once

#include <cstdint>
#include <iostream>

using namespace std;

namespace entity {

enum DistMetricMode {
  FULL, // full PSD matrix
  DIAG, // diagonal matrix
  EDIAG // diagonal matirx with tied diagonal elements
};

}  // namespace entity
