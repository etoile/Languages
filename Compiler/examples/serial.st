NSObject subclass: SmalltalkTool [
	run [
		'Passing self in a block...' log.
		1 ifTrue: [ self wibble. (NSObject new) log. ].
	]

	wibble [
		'Calling self method in block' log.
	]
]
