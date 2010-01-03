NSObject subclass: SmalltalkTool [

	do:b [ b value: 'wibble'. ] 
	run [
	| ret |
		self do:[ :x | 1 ifTrue:[ x log. ]. ] . 
		ret := 'Ret incorrectly set'.
		self do:[ :x | ret := x. ].
		ret log.	
	]
]
