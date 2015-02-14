#!/bin/bash

DATA_PATH=/home/zhitingh/ml_proj/EEEL/data/whole_new/

# 1) add number_of_fileds to the biginning of each line
# 2) change : to <space> 
cat ${DATA_PATH}/entity2ancestor.txt | awk '{printf("%d\t%s\n", NF, $0)}' | sed 's/:/ /g' > ${DATA_PATH}/new_entity2ancestor.txt
