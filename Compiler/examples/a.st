NSObject subclass: SmalltalkTool
[
	run [
	| a |
		[ :x | x log. ] value: 'foo'.
]	
]
