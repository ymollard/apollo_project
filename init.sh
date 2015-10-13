#!/bin/bash

export HEADAS=`pwd`/heasoft-6.17/x86_64-unknown-linux-gnu-libc2.5/BUILD_DIR
export SAS=`pwd`/ctadev

export HEADSOFT=$HEADAS
export SAS_PATH=$SAS
source $HEADAS/headas-init.sh
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$HEADAS/../lib/:$SAS/lib
