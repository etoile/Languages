NSObject subclass: SmalltalkTool [
	| f |

	run [
		self fibonacci: 30.
	]
	fibonacci: n [
		| c ret |
		c := (n < 2).
		c ifTrue:[
			ret := 1.
		] ifFalse:[
			ret := (self fibonacci:(n - 1))
				 + (self fibonacci:(n - 2)).
		].
		^ret.
	]
]
