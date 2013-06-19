#!/bin/bash

cd ./libs/table-4.3.0phmodified
make
cp table.o ../

cd ../
make

cd ../
make

mkdir $1
cp afserverd $1

cd $1
touch POSID.cfg 
echo "0" > POSID.cfg
