NSObject subclass: Super [
	dealloc [ ETTranscript show: 'Super destroyed' ; cr ]
]
Super subclass: Sub [
	dealloc [ ETTranscript show: 'Sub destroyed' ; cr ]
]
NSObject subclass: SmalltalkTool
[
	run [ Sub new ]
]
