NSObject subclass: SmalltalkTool [
	stringForRect: rect
	[
		^ '{x = ', (rect x stringValue), '; y = ', (rect y stringValue), 
		  '; width = ', (rect width stringValue), '; height = ', (rect height stringValue), '}'.
	]


	run [ | rect |
		rect := NSValue rectWithX: '1.2' Y: '2.7' width: '3.032' height: '-5000000000.11'.

		ETTranscript
			show: 'Boxing:'; cr;
			show: (self stringForRect: (rect rectValue)); cr;
			show: 'Unboxing:'; cr;
			show: ((NSValue valueWithRect: rect) isEqualToValue: rect); cr.
	]
]
