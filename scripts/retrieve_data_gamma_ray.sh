#!/bin/bash
init=`pwd`
data_location=http://nssdcftp.gsfc.nasa.gov/spacecraft_data/apollo/apollo15_csm/gamma-ray_spectrometer/gamma-ray_spectrometer_merge_data/
cd ../data
wget -r -l1 -nd --no-parent $data_location

for file in *.tar;
do
    tar xf $file
    rm -rf $file
done

mv */*.DAT .

for dir in *.dir;
do
    rmdir $dir
done

rm *.htm*
cd $init
