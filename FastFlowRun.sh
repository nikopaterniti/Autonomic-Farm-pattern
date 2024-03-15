#!/bin/bash

# Build the source
make prodff 

# The number of CSV generated for each input
runs_number=2
expected_troughput=5
initial_nw=64

echo "Running FastFlow with $initial_nw nw and $expected_troughput throughput"
for (( i=1; i<=$runs_number; i++ ))
	do  
		echo "srv-ff-default-$i-$initial_nw-$expected_troughput.csv..."
		./farm-ff.out $initial_nw $expected_troughput > csv/srv-ff-default-$i-$initial_nw-$expected_troughput.csv

		echo "srv-ff-constant-$i-$initial_nw-$expected_troughput.csv..."
		./farm-ff.out $initial_nw $expected_troughput 2 > csv/srv-ff-constant-$i-$initial_nw-$expected_troughput.csv

		echo "srv-ff-reverse-default-$i-$initial_nw-$expected_troughput.csv..."
		./farm-ff.out $initial_nw $expected_troughput 3 > csv/srv-ff-reverse-default-$i-$initial_nw-$expected_troughput.csv

		echo "srv-ff-lowhigh-$i-$initial_nw-$expected_troughput.csv..."
		./farm-ff.out $initial_nw $expected_troughput 4 > csv/srv-ff-lowhigh-$i-$initial_nw-$expected_troughput.csv
 
		echo "srv-ff-highlow-$i-$initial_nw-$expected_troughput.csv..."
		./farm-ff.out $initial_nw $expected_troughput 5 > csv/srv-ff-highlow-$i-$initial_nw-$expected_troughput.csv
	done

:'
./farm-ff.out 128 20 > csv/srv-ff-default-1-128-20.csv
./farm-ff.out 128 20 > csv/srv-ff-default-2-128-20.csv

./farm-ff.out 128 20 2 > csv/srv-ff-constant-1-128-20.csv
./farm-ff.out 128 20 2 > csv/srv-ff-constant-2-128-20.csv

./farm-ff.out 128 20 3 > csv/srv-ff-reverse-default-1-128-20.csv
./farm-ff.out 128 20 3 > csv/srv-ff-reverse-default-2-128-20.csv

./farm-ff.out 128 20 4 > csv/srv-ff-lowhigh-1-128-20.csv
./farm-ff.out 128 20 4 > csv/srv-ff-lowhigh-2-128-20.csv

./farm-ff.out 128 20 5 > csv/srv-ff-highlow-1-128-20.csv
./farm-ff.out 128 20 5 > csv/srv-ff-highlow-2-128-20.csv

./farm.out 1 20 2 > csv/srv-constant-1-1-20.csv
./farm.out 1 20 3 > csv/srv-reverse-default-1-1-20.csv
'

