
NSObject subclass: SmalltalkTool
[
	run [
		ETTranscript show: 'NSNotFound is: '; show: (C enumValue: NSNotFound) ; cr.
	]
]
