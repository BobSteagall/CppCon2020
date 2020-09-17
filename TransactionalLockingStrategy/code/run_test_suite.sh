#! /bin/bash

# set -x

tx_count=100000
cpu_count=24

mkdir -p r04
../run_tests.sh -d r04 -i 10000 -x $tx_count -r 20 -m 0                | tee r04/results_04.txt
../run_tests.sh -d r04 -i 10000 -x $tx_count -r 20 -c $cpu_count -m 1  | tee -a r04/results_04.txt
../run_tests.sh -d r04 -i 10000 -x $tx_count -r 20 -c $cpu_count -m 2  | tee -a r04/results_04.txt
../run_tests.sh -d r04 -i 10000 -x $tx_count -r 20 -c $cpu_count -m 3  | tee -a r04/results_04.txt

mkdir -p r05
../run_tests.sh -d r05 -i 100000 -x $tx_count -r 20 -m 0                | tee   r05/results_05.txt
../run_tests.sh -d r05 -i 100000 -x $tx_count -r 20 -c $cpu_count -m 1  | tee -a r05/results_05.txt
../run_tests.sh -d r05 -i 100000 -x $tx_count -r 20 -c $cpu_count -m 2  | tee -a r05/results_05.txt
../run_tests.sh -d r05 -i 100000 -x $tx_count -r 20 -c $cpu_count -m 3  | tee -a r05/results_05.txt

mkdir -p r06
../run_tests.sh -d r06 -i 1000000 -x $tx_count -r 20 -m 0                | tee   r06/results_06.txt
../run_tests.sh -d r06 -i 1000000 -x $tx_count -r 20 -c $cpu_count -m 1  | tee -a r06/results_06.txt
../run_tests.sh -d r06 -i 1000000 -x $tx_count -r 20 -c $cpu_count -m 2  | tee -a r06/results_06.txt
../run_tests.sh -d r06 -i 1000000 -x $tx_count -r 20 -c $cpu_count -m 3  | tee -a r06/results_06.txt

mkdir -p r07
../run_tests.sh -d r07 -i 10000000 -x $tx_count -r 20 -m 0                | tee   r07/results_07.txt
../run_tests.sh -d r07 -i 10000000 -x $tx_count -r 20 -c $cpu_count -m 1  | tee -a r07/results_07.txt
../run_tests.sh -d r07 -i 10000000 -x $tx_count -r 20 -c $cpu_count -m 2  | tee -a r07/results_07.txt
../run_tests.sh -d r07 -i 10000000 -x $tx_count -r 20 -c $cpu_count -m 3  | tee -a r07/results_07.txt
