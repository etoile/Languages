NSObject subclass: TestObject
[
	whatever [ ^ nil. ]
	whichever [ ^ 'something'. ]
]
NSObject subclass: SmalltalkTool
[
	run
	[
		ETTranscript show: (TestObject new valueForKey: 'whatever') ; cr;
			show: (TestObject new valueForKey: 'whichever') ; cr.
	]
]
