#!/usr/bin/env bash

# Figure out the paths.
script_path=`readlink -f $0`
script_dir=`dirname $script_path`
app_dir=`dirname $script_dir`
progname=ee_main
prog_path=${app_dir}/build/tools/${progname}

# Data
dataset_name=whole
#dataset_name=tech_parsed
#dataset_name=apple
dataset_path="${app_dir}/../EEEL/data/${dataset_name}"

## Parameters
# embedding
dim_embedding=100;
distance_metric_mode="DIAG";

# training engine parameters
num_iter=76000 # = 1 epoches
eval_interval=800
num_iter_per_eval=20
batch_size=500

# solver parameters
learning_rate=50
num_neg_sample=10
num_epoch_on_batch=1
num_iter_on_entity=1
num_iter_on_category=1
snapshot=16000
#resume_path="/home/zhitingh/ml_proj/EEEL_dim100/output/eeel_whole_parsed_D100_MDIAG_lr0.01_N10_B500-8000"
#resume_iter=40000

# Output
output_dir=${app_dir}/output
output_dir="${output_dir}/eeel_${dataset_name}_D${dim_embedding}_M${distance_metric_mode}"
output_dir="${output_dir}_lr${learning_rate}_N${num_neg_sample}_B${batch_size}-rectify"
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
    --learning_rate $learning_rate \
    --num_neg_sample $num_neg_sample \
    --num_epoch_on_batch $num_epoch_on_batch \
    --num_iter_on_entity $num_iter_on_entity \
    --num_iter_on_category $num_iter_on_category \
    --dataset_path $dataset_path \
    --output_file_prefix $output_dir \
    --snapshot $snapshot \
    #--resume_path $resume_path \
    #--resume_iter $resume_iter
