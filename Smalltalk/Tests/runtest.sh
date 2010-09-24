#!/bin/sh

EXIT=0
SCRIPT=$1
shift
cd ${SCRIPT}
echo "------------------------------------------------------"
echo "Test: ${SCRIPT} $@"
if (edlc $@ -f test.st > results.txt 2>/dev/null) ; then
	if [ -f results.txt ]; then
		if cmp results.txt expected.txt > /dev/null; then
			echo -e "\033[0;32m${SCRIPT}: OK\033[m"
		else
			echo -e "\033[0;31m${SCRIPT}: FAIL\033[m"
			EXIT=1
			echo 'result | expected'
			sdiff results.txt expected.txt
		fi
		rm results.txt
	else
		EXIT=2
		echo -e "\033[0;31m${SCRIPT}: FAIL (gave no output)\033[m"
	fi
else
	EXIT=3
	echo -e "\033[0;33m${SCRIPT}: FAIL (crash)\033[m"
fi
cd ..
exit $EXIT
