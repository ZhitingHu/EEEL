#!/usr/bin/env bash

# Figure out the paths.
script_path=`readlink -f $0`
script_dir=`dirname $script_path`
app_dir=`dirname $script_dir`
progname=ee_main
prog_path=${app_dir}/build/tools/${progname}

# Data
dataset_name=apple
dataset_path="${app_dir}/../../EEEL/data/${dataset_name}"

## Parameters
# embedding
dim_embedding=100;
distance_metric_mode="DIAG";

# training engine parameters
num_iter=100 # = 3 epoches
eval_interval=1
num_iter_per_eval=5
batch_size=100

# solver parameters
#solver_type="ADAGRAD"
solver_type="MOMEN"
#solver_type="SGD"
#momentum=0.9
learning_rate=0.1
num_neg_sample=50
num_epoch_on_batch=1
num_iter_on_entity=1
num_iter_on_category=1
snapshot=1000
#
#resume_path="./output/eeel_apple_D100_MDIAG_lr0.01_N50_B100"
#resume_iter=50

# Output
output_dir=${app_dir}/output
output_dir="${output_dir}/eeel_${dataset_name}_D${dim_embedding}_M${distance_metric_mode}"
output_dir="${output_dir}_lr${learning_rate}_N${num_neg_sample}_B${batch_size}_S${solver_type}"
#rm -rf ${output_dir}
mkdir -p ${output_dir}
log_dir=${output_dir}/logs
mkdir -p ${log_dir}

# Run
echo Running ee_main

GLOG_logtostderr=0 \
GLOG_stderrthreshold=0 \
GLOG_log_dir=$log_dir \
GLOG_v=-1 \
GLOG_minloglevel=0 \
GLOG_vmodule="" \
    $prog_path \
    --dim_embedding $dim_embedding \
    --distance_metric_mode $distance_metric_mode \
    --num_iter $num_iter \
    --eval_interval $eval_interval \
    --num_iter_per_eval $num_iter_per_eval \
    --batch_size $batch_size \
    --solver_type $solver_type \
    --learning_rate $learning_rate \
    --num_neg_sample $num_neg_sample \
    --num_epoch_on_batch $num_epoch_on_batch \
    --num_iter_on_entity $num_iter_on_entity \
    --num_iter_on_category $num_iter_on_category \
    --dataset_path $dataset_path \
    --output_file_prefix $output_dir \
    --snapshot $snapshot #\
    #--resume_path $resume_path \
    #--resume_iter $resume_iter
