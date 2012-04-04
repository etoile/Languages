NSObject subclass: SmalltalkTool [
	run [
		('a' characterAtIndex: 0) isAlphabetic ifTrue:
		 [ETTranscript show: 'true'; cr]
			ifFalse: [ETTranscript show: 'false'; cr]. 
	]
]
