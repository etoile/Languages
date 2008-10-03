#!/bin/sh
LD_LIBRARY_PATH=`pwd`/SmalltalkKit/SmalltalkKit.framework/Versions/Current:$LD_LIBRARY_PATH gdb ./obj/st
