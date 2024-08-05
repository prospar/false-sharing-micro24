#!/bin/bash

#bash ./run-benchmark.sh MESI_Two_Level false-sharing
#bash ./run-benchmark.sh MESI_Two_Level false-true-sharing-naive
#bash ./run-benchmark.sh MESI_Two_Level true-false-sharing
#bash ./run-benchmark.sh MESI_Two_Level true-sharing
#bash ./run-benchmark.sh MESI_Two_Level without-false-sharing

#bash ./run-benchmark.sh MESI_Two_Level_Extended false-sharing
#bash ./run-benchmark.sh MESI_Two_Level_Extended false-true-sharing-naive
#bash ./run-benchmark.sh MESI_Two_Level_Extended true-false-sharing
#bash ./run-benchmark.sh MESI_Two_Level_Extended true-sharing
#bash ./run-benchmark.sh MESI_Two_Level_Extended without-false-sharing

#bash ./run-benchmark.sh FS_MESI_DETECTION false-sharing
#bash ./run-benchmark.sh FS_MESI_DETECTION false-true-sharing-naive
#bash ./run-benchmark.sh FS_MESI_DETECTION true-false-sharing
#bash ./run-benchmark.sh FS_MESI_DETECTION true-sharing
#bash ./run-benchmark.sh FS_MESI_DETECTION without-false-sharing

#bash ./run-benchmark.sh FS_MESI false-sharing
#bash ./run-benchmark.sh FS_MESI false-true-sharing-naive
#bash ./run-benchmark.sh FS_MESI true-false-sharing
#bash ./run-benchmark.sh FS_MESI true-sharing
#bash ./run-benchmark.sh FS_MESI without-false-sharing


bash ./run-benchmark.sh FS_MESI_Blocking true-sharing
bash ./run-benchmark.sh FS_MESI_Blocking true-false-sharing
bash ./run-benchmark.sh FS_MESI_Blocking false-sharing
bash ./run-benchmark.sh FS_MESI_Blocking false-true-sharing-naive
bash ./run-benchmark.sh FS_MESI_Blocking false-sharing

