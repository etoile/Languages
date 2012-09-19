#!/bin/sh

#Generate bitcode for all of the Smalltalk files
for ST in $@ 
do
	echo Compiling $ST to LLVM bitcode...
	edlc -c -f $ST -v LKLowerIfTrueTransform
done

# Link the bitcode together
BC=`echo $@ | sed 's/\.st/\.bc/g'`
echo Linking bitcode files...
llvm-link -f -o smalltalk.bc \
	${GNUSTEP_LOCAL_ROOT}/Library/Frameworks/LanguageKitCodeGen.framework/Versions/0/Resources/MsgSendSmallInt.bc\
	$BC

# Optimise the bitcode
echo Running optimiser...
opt -load /usr/local/lib/libGNUObjCRuntime.so -objc-arc-expand -objc-arc -objc-arc-aa    -objc-arc-apelim   -objc-arc-contract -O3 smalltalk.bc -o smalltalk.optimised.bc
opt -objc-arc-expand -objc-arc -objc-arc-aa    -objc-arc-apelim   -objc-arc-contract -O3 smalltalk.bc -o smalltalk.optimised.bc

# Generate assembly
echo Compiling bitcode...
llc smalltalk.optimised.bc

# Assemble a .o file
echo Assembling object file...
gcc -c -g smalltalk.optimised.s 
