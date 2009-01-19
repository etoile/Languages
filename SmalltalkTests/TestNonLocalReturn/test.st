
NSObject subclass: SmalltalkTool [
	
	ignoresReturn: aBlock
	[
		aBlock value.
		^'test failed'.
	]
	
	returnsObject
	[
		self ignoresReturn:[  ^'test succeeded' ].
		"1 ifTrue: [ ^'test succeeded'] ."
		^'test failed'
	]

	run [
		ETTranscript show: self returnsObject; cr.

	]
]
