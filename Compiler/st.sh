#!/bin/sh
#
# test.sh : Small script for testing SmalltalkKit
#
#gmake  --no-print-directory debug=no
LD_LIBRARY_PATH=`pwd`/SmalltalkKit/SmalltalkKit.framework/Versions/Current:\
`pwd`/Support/obj:$LD_LIBRARY_PATH ./obj/st $@
