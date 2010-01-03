NSObject subclass: SmalltalkTool [

	do:b [ b value: 'wibble'. ] 
	run [
		self do:[ :x | 1 ifTrue:[ x log. ]. ] . 
	]
]
