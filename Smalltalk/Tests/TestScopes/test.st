
NSObject subclass: SmalltalkTool [
	| +aClassVar anIvar |
	 run [
		| methodLocal |
		methodLocal := 'null'.
		aClassVar := 'null'.
		anIvar := 'null'.
		self secondMethod.
		ETTranscript show: methodLocal; cr.
		ETTranscript show: anIvar; cr.
		ETTranscript show: aClassVar; cr.
		[
			| blockLocal |
			[
				methodLocal := 'set in inner block'.
				blockLocal := 'set in inner block'.
				aClassVar := 'set in inner block'.
				anIvar := 'set in inner block'.
			] value.
			ETTranscript show: methodLocal; cr.	
			ETTranscript show: blockLocal; cr.
			ETTranscript show: aClassVar; cr.
			ETTranscript show: anIvar; cr.
		] value.

		self thirdMethod: [
			ETTranscript show: methodLocal; cr.
		].
	 ]

	secondMethod [
		| methodLocal |
		methodLocal := 'null2'.
		ETTranscript show: methodLocal; cr.
		anIvar := 'set in secondMethod'.
		aClassVar := 'set in secondMethod'.
	]

	thirdMethod: block [
		| methodLocal |
		methodLocal := 'should never see!'.
		block value.
	]
]
