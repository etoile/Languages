NSObject subclass: LS
[
	run
	[
		| fm files out |
		fm := NSFileManager defaultManager.
		files := fm directoryContentsAtPath:'.'.
		out := NSFileHandle fileHandleWithStandardOutput.
		files foreach:[ :x | x printOn:out. '\n' printOn:out. ].
	]
]
