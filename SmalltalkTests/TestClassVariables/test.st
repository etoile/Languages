
NSObject subclass: Thing [
	 | +instances |
	 
	 +initialize [
	       ETTranscript show: 'Class initialisation'; cr.
	       instances := NSMutableArray new.
	 ]

	 init [
	       instances addObject: self.
	       ETTranscript show: 'Object initialisation (';
	       		    show: instances ; show: ')'; cr.
	       ^ self.
	 ]
	 
]

NSObject subclass: SmalltalkTool [
	 run [
	     | a b c |
	     a := Thing new.
	     b := Thing new.
	     c := Thing new.
	 ]
]
