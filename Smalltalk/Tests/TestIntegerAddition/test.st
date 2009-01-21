
NSObject subclass: SmalltalkTool [
	 run [
	     | a b |
	     a := 4.
	     b := 6.
	     ETTranscript show: 'Integer addition:'; cr;
	     		  show: (1 + 2); cr;
			  show: (a + 1); cr;
	     		  show: (1 + a); cr;
			  show: (a + b); cr.
	 ]
]
