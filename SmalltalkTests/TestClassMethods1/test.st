

NSObject subclass: SmalltalkTool [
	 +show: string [
	     ETTranscript show: string.
	     ETTranscript cr.
	 ]
	 
	 run [
	     SmalltalkTool show: 'simple class method invocation'.
	     self class show: 'simple class method invocation 2'.
	 ]
]
