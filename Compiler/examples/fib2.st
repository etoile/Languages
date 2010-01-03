NSObject subclass: SmalltalkTool [
	| f |

	init [
		f := NSMutableArray new.
		f addObject:1.
		f addObject:1.
		^self.
	]
	run []
	fibonacci: n [
		| ret |
		(n < (f count)) 
		 ifTrue:[
			ret := f objectAtIndex:n.
		] ifFalse:[
			ret := (self fibonacci:(n - 1))
				 + (self fibonacci:(n - 2)).
			f insertObject:ret atIndex:n.
		].
		ret log.
		^ret.
	]

	intValue [ 
	| a |
		a := 6.
		a := a + 7.
^a. ]
]
