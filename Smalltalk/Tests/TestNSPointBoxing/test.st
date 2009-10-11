NSObject subclass: SmalltalkTool [
	stringForPoint: point
	[
		^ '{x = ', (point x stringValue), '; y = ', (point y stringValue), '}'.
	]

	run [ | point |
		point := NSValue pointWithX: '1.1' Y: '10.2'.
		ETTranscript
			show: 'Boxing:'; cr;
			show: (self stringForPoint: (point pointValue)); cr;
			show: 'Unboxing:'; cr;
			show: ((NSValue valueWithPoint: point) isEqualToValue: point); cr.
	]
]
