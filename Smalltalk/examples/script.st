NSObject subclass: SmalltalkTool [
	run [
	" Run this with -l ScriptKit.  It will close the active Typewriter window"
		
		Tell application:'Typewriter' to:[ :d |
			(d objectForKey:'Application') sendAction:#performClose: 
					to:nil 
					from:nil .
		] .
	]
]
