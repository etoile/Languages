NSObject subclass: SmalltalkTool [
	run [
		|str |
		str := 'foo'.
		str log.
		str := str stringByAppendingString:'bar'.
		str log.
		str := 'foo' mutableCopy.
		str log.
		str appendString:'bar'.
		str log.
	]
]
