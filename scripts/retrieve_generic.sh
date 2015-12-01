#!/bin/bash
if [ $# != 1 ];
then
    >&2 echo "This script downloads and untars all *.tar archives at a specific url"
    >&2 echo "USAGE: "$0" <url_to_tar_archives>"
    exit 1
fi

init=`pwd`
data_location=$1

mkdir -p ../data
cd ../data
wget -r -l1 -nd --no-parent -e robots=off $data_location

for file in *.tar;
do
    tar xf $file
    if [ $? == 0 ];
    then
        rm $file
    fi
done

mv */*.DAT .

for dir in *.dir;
do
    rmdir $dir
done

rm *.htm* robots.txt SHA1SUM 2> /dev/null
cd $init
