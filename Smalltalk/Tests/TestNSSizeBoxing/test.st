NSObject subclass: SmalltalkTool [
	run [ | size |
		size := NSValue sizeWithWidth: '13.1' height: '50.5'.
		ETTranscript
			show: 'Boxing:'; cr;
			show: size sizeValue; cr;
			show: 'Unboxing:'; cr;
			show: ((NSValue valueWithSize: size) isEqualToValue: size); cr.
	]
]
