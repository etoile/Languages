
NSObject subclass: Thing [
	 | +intClassVariable |
	 
	 +initialize [
	       ETTranscript show: 'Class initialisation'; cr.
	       intClassVariable := 42.
	 ]

	 init [
	       ETTranscript show: 'Object initialisation (';
	       		    show: intClassVariable ; show: ')'; cr.
	       ^ self.
	 ]
	 
]

NSObject subclass: SmalltalkTool [
	 run [
	     | a |
	     a := Thing new.
	 ]
]
