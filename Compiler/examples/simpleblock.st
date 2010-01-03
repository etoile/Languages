NSObject subclass: SmalltalkTool [

	do:b [ b value: 'wibble'. ] 

	run [
	| return |
		self do:[ :x | return := x.  ].
		return log.	
	]
]
