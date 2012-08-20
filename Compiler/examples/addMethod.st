NSObject subclass: SmalltalkTool [
	run [
		| count |
		ETTranscript show: (self class) ; cr.
		count := 0.
		self class addInstanceMethod: #testMethod fromBlock:
			[ :self |
				count := count + 1.
				ETTranscript show: count ; show: self ;  cr
			].
		self testMethod.
		self testMethod.
		self testMethod
	]
]
