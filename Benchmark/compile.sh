#!/bin/sh

#Generate bitcode for all of the Smalltalk files
for ST in $@ 
do
	edlc -c -f $ST -v LKLowerIfTrueTransform
done

# Link the bitcode together
BC=`echo $@ | sed 's/\.st/\.bc/g'`
llvm-link -f -o smalltalk.bc \
	${GNUSTEP_LOCAL_ROOT}/Library/Frameworks/LanguageKitCodeGen.framework/Versions/0/Resources/MsgSendSmallInt.bc\
	$BC

# Optimise the bitcode
opt -f -O3 smalltalk.bc -o smalltalk.optimised.bc

# Generate assembly
llc -f smalltalk.optimised.bc

# Assemble a .o file
gcc -c smalltalk.optimised.s 
