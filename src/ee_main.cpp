#include "ee_engine.hpp"
#include "context.hpp"
//#include <gflags/gflags.h>
//#include <glog/logging.h>
#include <vector>
#include <cstdint>

// entity embedding parameters
//DEFINE_int32(dim_entity_vector, 100, "");
//DEFINE_int32(distance_metric_mode, 0, "");
//DEFINE_string(dataset_path, "", "data path");
//DEFINE_int32(num_epochs, 1, "Number of data sweeps.");
//DEFINE_int32(batch_size, 50, "");
//DEFINE_int32(num_neg_sample, 50, "");
//DEFINE_double(learning_rate, 0.1, "Initial step size");
//DEFINE_int32(num_batches_per_eval, 10, "Number of batches per evaluation");

// Misc
//DEFINE_string(output_file_prefix, "", "Results go here.");


int main(int argc, char *argv[]) {
  //google::ParseCommandLineFlags(&argc, &argv, true);
  //google::InitGoogleLogging(argv[0]);

  entity::EEEngine ee_engine;
  //ee_engine.ReadData();
  //ee_engine.Start();

  //LOG(INFO) << "Entity Embedding finished and shut down!";
  std::cout << "Entity Embedding finished and shut down!" << std::endl;
  return 0;
}
