
NSObject subclass: Thing [
	 | +instances |
	 
	 +initialize [
	       ETTranscript show: 'Class initialisation'; cr.
	       instances := NSMutableString new.
	 ]

	 init [
	       instances appendString: 'a'.
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
