
NSObject subclass: SmalltalkTool [
	 run [
		| factorial |
	
		" Inspired by http://forums.newspeaklanguage.org/index.php?topic=23.0"
	
		factorial := [ :x | 
		       x <= 0 ifTrue: [ 1 ]
			     ifFalse: [ x * (factorial value: (x - 1)) ]
		].
		ETTranscript show: (factorial value: 5); cr.
	 ]
]
