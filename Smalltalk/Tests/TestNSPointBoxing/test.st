NSObject subclass: SmalltalkTool [
	run [ | point |
		point := NSValue pointWithX: '1.1' Y: '10.2'.
		ETTranscript
			show: 'Boxing:'; cr;
			show: point pointValue; cr;
			show: 'Unboxing:'; cr;
			show: ((NSValue valueWithPoint: point) isEqualToValue: point); cr.
	]
]
