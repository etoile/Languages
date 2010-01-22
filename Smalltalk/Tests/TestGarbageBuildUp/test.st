NSObject subclass: SmalltalkTool [
	makeGarbage [
		| garbage |
		garbage := NSMutableString alloc initWithCapacity: 1000000.
	]

	run [
		ETTranscript show: 'starting'; cr.
		1 to: 1000000 do: [ self makeGarbage ].
		ETTranscript show: 'done'; cr.
	]
]
