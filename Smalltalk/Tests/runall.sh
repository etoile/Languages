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

PASSES=0
FAILS=0
# run the tests contained in the Test* directories
for i in Test*; do
	if [ 1 -eq $QUIET ] ; then
		if (sh runtest.sh $i > /dev/null 2>&1) ; then
			PASSES=`expr $PASSES + 1`
			echo -n .
		else
			FAILS=`expr $FAILS + 1`
			echo -n '{FAIL: '$i'}'
		fi	
	else
    	sh runtest.sh $i
	fi
done
if [ 1 -eq $QUIET ] ; then
	echo
	echo `expr $PASSES + $FAILS` tests run.  $PASSES passed, $FAILS failed.
fi
rm -f */*.core
exit $FAILS
