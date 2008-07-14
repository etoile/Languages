#!/bin/sh
touch main.m
gmake debug=yes
LD_LIBRARY_PATH=`pwd`/SmalltalkKit/obj:$LD_LIBRARY_PATH gdb ./obj/st
