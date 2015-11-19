#!/bin/bash

init=`pwd`
cd ../src

failures=""
for file_id in `seq 14190 14196`;
do
    file="../data/DD0"$file_id"_F1.DAT"
    ./main_mass inputfile=$file
    if [ $? != 0 ];
    then
        failures="$failures $file"
        >&2 echo $file FAILED to convert
    fi
done
echo "Processing complete, errors:" $failures
cd $init
