#ifndef ENTITY_COMMON_HPP_
#define ENTITY_COMMON_HPP_

#include "context.hpp"

#include <stdint.h>
#include <iostream>
#include <glog/logging.h>

#ifndef GFLAGS_GFLAGS_H_
namespace gflags = google;
#endif  // GFLAGS_GFLAGS_H_
using namespace std;

namespace entity {

extern double expand_time;
extern double find_time;

}  // namespace entity

#endif
