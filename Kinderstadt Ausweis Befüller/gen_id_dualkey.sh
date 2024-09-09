#!/bin/bash

make

echo "Enter name of the group input csv file"
read group_in

echo "Enter name of the input csv file"
read in

echo "Enter name of the output file (without spaces and without .pdf)"
read out

echo "Enter name of the role key in the csv file (without spaces and without quotes)"
read role

echo "Enter name of the prename key in the csv file (without spaces and without quotes)"
read prename

echo "Enter name of the aftername key in the csv file (without spaces and without quotes)"
read aftername

./pipe_inputer_dualkey.run $group_in $in $role $prename $aftername | ./programm.run $out 1

make clean