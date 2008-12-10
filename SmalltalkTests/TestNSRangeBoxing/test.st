NSObject subclass: SmalltalkTool [
	run [ | range |
		range := (NSValue rangeWithLocation:0 length:9).
		ETTranscript 
			show:'Create range: '; cr;
			show: range; cr;
			show: ('Substring of longer string' substringWithRange:range); cr.
	]
]
