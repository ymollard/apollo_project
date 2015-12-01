#!/bin/bash

init=`pwd`
cd ../src/mass_spectrometer

failures=""
for file_id in `seq 14190 14196`;
do
    file="../../data/DD0"$file_id"_F1.DAT"
    ./main_mass inputfile=$file
    if [ $? != 0 ];
    then
        failures="$failures""\n""$file"
        >&2 echo $file FAILED to convert
    fi
done

if [ "$failures" == "" ];
then
    echo "Processing complete without error!"
else
    >&2 echo -e "Processing complete, errors occured on the following files:" $failures
fi
cd $init
