#!/bin/bash

init=`pwd`
cd ../src

failures=""
for file in ../data/*.DAT;
do
    ./main_gamma_ray inputfile=$file
    if [ $? != 0 ];
    then
        failures="$failures $file"
        >&2 echo $file FAILED to convert
    fi
done
echo "Processing complete, errors:" $failures
cd $init
