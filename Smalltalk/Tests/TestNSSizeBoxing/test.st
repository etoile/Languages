NSObject subclass: SmalltalkTool [
	stringForSize: size
	[
		^ '{width = ', (size width stringValue), '; height = ', (size height stringValue), '}'.
	]
	
	run [ | size |
		size := NSValue sizeWithWidth: '13.1' height: '50.5'.
		ETTranscript
			show: 'Boxing:'; cr;
			show: (self stringForSize: (size sizeValue)); cr;
			show: 'Unboxing:'; cr;
			show: ((NSValue valueWithSize: size) isEqualToValue: size); cr.
	]
]
