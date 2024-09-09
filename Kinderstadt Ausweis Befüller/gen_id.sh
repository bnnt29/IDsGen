#!/bin/bash

make

echo "Enter name of the input csv file"
read in

echo "Enter name of the output file (without spaces and without .pdf)"
read out

echo "Enter name of the key in the csv file (without spaces and without quotes)"
read names

echo "Enter name of the Group"
read group

echo "Enter red color of the Group (0-255)"
read red

echo "Enter green color of the Group (0-255)"
read green

echo "Enter blue color of the Group (0-255)"
read blue

./pipe_inputer.run $in $names $group $red $green $blue | ./programm.run $out 1

make clean