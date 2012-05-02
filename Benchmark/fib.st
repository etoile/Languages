<< types = '{"fibonacci:" = "i12@0:4i8";}'>>
NSObject subclass: SmalltalkFibonacci [
	| f |

	fib: n [
		| c ret |
		c := (n < 2).
		c ifTrue:[
			ret := 1.
			nil.
		] ifFalse:[
			ret := (self fib:(n - 1))
				 + (self fib:(n - 2)).
			nil.
		].
		^ret.
	]
	fibonacci: n [
		| c ret |
		c := (n < 2).
		c ifTrue:[
			ret := 1.
			nil.
		] ifFalse:[
			ret := (self fibonacci:(n - 1))
				 + (self fibonacci:(n - 2)).
			nil.
		].
		^ret.
	]
	runNative: i [ (self fib: i) ]
	run [
		"(self fibonacci:40) log."
		(self fib:40) log.
	]
]
