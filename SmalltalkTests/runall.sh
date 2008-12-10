#!/usr/bin/env bash

QUIET=0
set -- `getopt q "$@"`
while [ $# -gt 0 ]
do
    case "$1" in
		-q)	QUIET=1; break;;
		--)	shift; break;;
		*)	break;;		
    esac
    shift
done

# run the tests contained in the Test* directories
for i in Test*; do
	if [ 1 -eq $QUIET ] ; then
		if (sh runtest.sh $i > /dev/null 2>&1) ; then
			echo -n .
		else
			echo -n '{FAIL: '$i'}'
		fi	
	else
    	sh runtest.sh $i
	fi
done
