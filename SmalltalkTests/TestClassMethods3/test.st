
ETTranscript subclass: MyTranscript [
	 +show: string [
	     super show: 'from subclass: '.
	     super show: string.
	 ]
]

NSObject subclass: SmalltalkTool [
	 run [
	     MyTranscript show: 'class method invocation'.
	     MyTranscript cr.
	 ]
]
