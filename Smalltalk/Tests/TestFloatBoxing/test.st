NSObject subclass: SmalltalkTool [
	run [
		| float |
		float := NSNumber numberWithFloat: '3.14159'.

		ETTranscript
			show: float; cr; 
      show: float floatValue; cr.
	]
]
