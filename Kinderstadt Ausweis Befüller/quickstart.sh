#!/bin/bash

make

echo "Roles: roles.txt"

echo "Input: Teilnehmer2023.csv"

echo "Out: out.pdf"

echo "Role key: Rolle"

echo "Prename key: Vorname"

echo "Lastname key: Nachname"

echo "Foto key: Foto"

./pipe_inputer_dualkey.run roles.txt Teilnehmer2023.csv Rolle Vorname Name Foto | ./programm.run out 1

make clean