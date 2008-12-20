NSObject subclass: SmalltalkPrototype
[
	value: a [ ^a. ]

	block
	[
		^ [ :x | x ].
	]

	makePrototype [
		| a |
		a := NSObject new.
		a becomePrototype.
		a setValue:[ :this :aValue | aValue ] forKey:'value:'.
		a setValue:[ :this :aValue | aValue log ] forKey:'logValue:'.
		a setValue:[ :this :aValue | ETTranscript show:aValue ]
		    forKey:'showValue:'.
		^a.
	]
]
