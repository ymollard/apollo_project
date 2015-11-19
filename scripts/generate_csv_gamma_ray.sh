#!/bin/bash

init=`pwd`
cd ../src/gamma_ray_spectrometer

failures=""
for file_id in `seq 19684 19730`;
do
    file="../../data/DD0"$file_id"_F1.DAT"
    ./main_gamma_ray inputfile=$file
    if [ $? != 0 ];
    then
        failures="$failures $file"
        >&2 echo $file FAILED to convert
    fi
done
echo "Processing complete, errors:" $failures
cd $init
