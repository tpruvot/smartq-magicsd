#!/bin/bash

CROSS=/usr/local/software/arm-2010q1-188-arm-gnueabi/bin/arm-none-linux-gnueabi-

export CROSS
make arch=arm mkSmartQ
