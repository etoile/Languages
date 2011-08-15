NSObject subclass: SmalltalkTool
[
	run 
	[
	| a |
		a := NSObject new.
		a becomePrototype.
		a setValue:[ :object :aValue | ETTranscript show:aValue; cr ] 
		    forKey:'logValue:'.
		a logValue:'A string'.
	]
]
