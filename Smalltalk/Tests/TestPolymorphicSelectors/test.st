NSObject subclass: SmalltalkTool [
	 run [
		| nsnumber int |
		int := NSURLProtectionSpace new port. "Returns an int, should be boxed to SmallInt"
		nsnumber := NSURL new port.	"Returns NSNumber (object)"
		ETTranscript show: (int class == nsnumber class).	"Should not crash"
	 ]
]
