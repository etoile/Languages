
NSObject subclass: SmalltalkTool [
	 run [
		| curr max |
		max := 5.
		curr := 0.
		[ curr < max ] whileTrue: [
			ETTranscript show: 'x'.
			curr := curr + 1.
		].
		ETTranscript cr.
	 ]
]
