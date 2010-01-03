NSObject subclass: SmalltalkTool
[
	run
	[
		|a|
		a := {'1'. '2'. '3'.}.
		a addObject:1.
(a objectAtIndex:3) log.
		a log.
		a count log.
	]
]

