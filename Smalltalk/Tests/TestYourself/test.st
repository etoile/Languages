
NSObject subclass: SmalltalkTool [
	 run [
		| anArray |
		anArray := NSMutableArray new
			addObject: 'c';
			addObject: 'a';
			addObject: 'b';
			sortUsingSelector: 'compare:';
			yourself. " so the array is the value of this message cascade expression"

		anArray foreach: [ :x | ETTranscript show: x; cr ].
	 ]
]
