
NSObject subclass: SmalltalkTool [
	 | three |
	 
	 run [
	     "
	     Tests whether integers can be used as instance variables.
	     
	     This breaks for me (Günther) on Ubuntu 8.10. Looks as if
	     three doesn't repond to the message. Possibly, the variable
	     doesn't change its value and stays nil.
	     "
	     three := 3.
	     
	     ETTranscript show: 'Santa:'.
	     three timesRepeat: [ ETTranscript show: ' Ho!'. ].
	     ETTranscript cr.
	 ]
]
