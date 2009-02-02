
NSObject subclass: SmalltalkTool [
	
	returnsObject
	[
		true ifTrue: [ ^'test succeeded'].
		^'test failed'
	]

	run [
		ETTranscript show: self returnsObject; cr.
	]
]
