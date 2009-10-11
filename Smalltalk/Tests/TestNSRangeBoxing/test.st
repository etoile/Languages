NSObject subclass: SmalltalkTool [
	stringForRange: r
	[
		^ '{location = ', (r location stringValue), '; length = ', (r length stringValue), '}'.
	]


	run [ | range |
		range := (NSValue rangeWithLocation:0 length:9).
		ETTranscript 
			show:'Create range: '; cr;
			show: (self stringForRange: range); cr;
			show: ('Substring of longer string' substringWithRange:range); cr.
	]
]
