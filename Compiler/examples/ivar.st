NSObject subclass: SmalltalkTool [
	| a |
	run [
		a := 1.
		true ifTrue: [ a log. a := 'wibble' ].
		a log.
	]
]
