<< types = '{"fibonacci:" = "i12@0:4i8"}'>>
NSObject subclass: SmalltalkFibonacci [
	| f |

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
]
