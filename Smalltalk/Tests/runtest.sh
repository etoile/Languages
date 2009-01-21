#!/bin/sh

EXIT=0
cd $1
echo "------------------------------------------------------"
echo "Test: $1"
if (edlc -f test.st > results.txt 2>/dev/null) ; then
	if [ -f results.txt ]; then
		if cmp results.txt expected.txt > /dev/null; then
			echo -e "\033[0;32m$1: OK\033[m"
		else
			echo -e "\033[0;31m$1: FAIL\033[m"
			EXIT=1
			echo 'result | expected'
			sdiff results.txt expected.txt
		fi
		rm results.txt
	else
		EXIT=2
		echo -e "\033[0;31m$1: FAIL (gave no output)\033[m"
	fi
else
	EXIT=3
	echo -e "\033[0;33m$1: FAIL (crash)\033[m"
fi
cd ..
exit $EXIT
