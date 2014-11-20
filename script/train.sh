#!/bin/bash

# Figure out the paths.
script_path=`readlink -f $0`
script_dir=`dirname $script_path`
app_dir=`dirname $script_dir`
progname=eeel_main
prog_path=${app_dir}/bin/${progname}

# Data
dataset_name=tech
dataset_path="${app_dir}/data/${dataset_name}"

## Parameters
# embedding
dim_embedding=100;
distance_metric_mode="DIAG";

# training engine parameters
num_epoch=1
num_batch_per_eval=1
num_batch_per_epoch=50
batch_size=50

# solver parameters
learning_rate=0.1
num_neg_sample=50
num_epoch_on_batch=1
num_iter_on_entity=1
num_iter_on_category=1
openmp=false

# Output
output_dir=${app_dir}/output
output_dir="${output_dir}/eeel_${dataset_name}_D${dim_embedding}_M${distance_metric_mode}"
output_dir="${output_dir}_lr${learning_rate}_N${num_neg_sample}_omp${openmp}"
rm -rf ${output_dir}
mkdir -p ${output_dir}
log_dir=${output_dir}/logs
mkdir -p ${log_dir}

# Run
echo Running eeel_main

GLOG_logtostderr=false \
GLOG_stderrthreshold=0 \
GLOG_log_dir=$log_dir \
GLOG_v=-1 \
GLOG_minloglevel=0 \
GLOG_vmodule="" \
    $prog_path \
    --dim_embedding $dim_embedding \
    --distance_metric_mode $distance_metric_mode \
    --num_epoch $num_epoch \
    --num_batch_per_eval $num_batch_per_eval \
    --num_batch_per_epoch $num_batch_per_epoch \
    --batch_size $batch_size \
    --learning_rate $learning_rate \
    --num_neg_sample $num_neg_sample \
    --num_epoch_on_batch $num_epoch_on_batch \
    --num_iter_on_entity $num_iter_on_entity \
    --num_iter_on_category $num_iter_on_category \
    --openmp $openmp \
    --dataset_path $dataset_path \
    --output_file_prefix $output_dir
