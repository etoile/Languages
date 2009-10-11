NSObject subclass: SmalltalkTool [
	run [ | rect |
		rect := NSValue rectWithX: '1.2' Y: '2.7' width: '3.032' height: '-5000000000.11'.

		ETTranscript
			show: 'Boxing:'; cr;
			show: rect rectValue; cr;
			show: 'Unboxing:'; cr;
			show: ((NSValue valueWithRect: rect) isEqualToValue: rect); cr.
	]
]
