NSObject subclass: SmalltalkTool [
	run [ | range |
		range := (NSValue rangeWithLocation:0 length:9).
		'Create range: ' log.
		range log.
		('Substring of longer string' substringWithRange:range) log.
	]
]
