
NSString extend [
	 plus: anotherString [
	       ^self stringByAppendingString: anotherString.
	 ]
]

NSObject subclass: SmalltalkTool [
	 run [
	     ETTranscript show: 'Urin' + 'stinkt'.
	     ETTranscript cr.
	 ]
]
