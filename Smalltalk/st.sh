#!/bin/sh
#
# test.sh : Small script for testing SmalltalkKit
#
LD_LIBRARY_PATH=`pwd`/SmalltalkKit/obj:$LD_LIBRARY_PATH ./obj/st $@
