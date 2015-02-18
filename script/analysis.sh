#!/usr/bin/env bash

# Figure out the paths.
script_path=`readlink -f $0`
script_dir=`dirname $script_path`
app_dir=`dirname $script_dir`
progname=ee_analysis_main
prog_path=${app_dir}/build/tools/${progname}

## Data
#dataset_name=whole
#dataset_name=tech
dataset_name=apple
dataset_path="${app_dir}/../../EEEL/data/${dataset_name}"

#resume_iter=880000
resume_iter=30

## Parameters
# embedding
dim_embedding=100;
distance_metric_mode="DIAG";

# solver parameters
learning_rate=0.2
num_neg_sample=5
batch_size=500

# Output
#output_dir="/home/zhitingh/ml_proj/EEEL_dim100_whole_min_ca/output/eeel_whole_D100_MDIAG_lr0.2_N5_B500-whole-min-ca-704000"
output_dir="/home/zhitingh/ml_proj/EEEL/output/eeel_apple_D100_MDIAG_lr0.01_N50_B100"
log_dir=${output_dir}/analysis_logs
mkdir -p ${log_dir}

# Run
echo Running eeel_analysis

GLOG_logtostderr=0 \
GLOG_stderrthreshold=0 \
GLOG_log_dir=$log_dir \
GLOG_v=-1 \
GLOG_minloglevel=0 \
GLOG_vmodule="" \
    $prog_path \
    --dim_embedding $dim_embedding \
    --distance_metric_mode $distance_metric_mode \
    --resume_path $output_dir \
    --resume_iter $resume_iter \
    --dataset_path $dataset_path \
    --output_file_prefix $output_dir \
    --num_neg_sample $num_neg_sample
