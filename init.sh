#!/bin/bash

export HEADAS=`pwd`/libs/heasoft-6.17/x86_64-unknown-linux-gnu-libc2.19-0
export SAS=`pwd`/libs/ctadev_64

export HEADSOFT=$HEADAS
export SAS_PATH=$SAS
source $HEADAS/headas-init.sh
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$HEADAS/lib:$SAS/lib
