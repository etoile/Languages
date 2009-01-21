
NSObject subclass: SmalltalkTool [
	 | two |
	 
	 run [
		" 2 becomes a BigInt when assigned to an ivar."
		two := 2.

		ETTranscript show: 'Santa:'.
		" Test BitInt"
		two timesRepeat: [ ETTranscript show: ' Ho!'. ].
		" Test SmallInt"
		1 timesRepeat: [ ETTranscript show: ' Ho!'. ].
		" Test BigInt negative arithmetic results with timesRepeat: "
		( two - 5) timesRepeat: [ ETTranscript show: ' Ho!'. ].
		" Test SmallInt negative arithmetic results with timesRepeat: "
		( 2 - 5) timesRepeat: [ ETTranscript show: ' Ho!'. ].
		ETTranscript cr.
	 ]
]
