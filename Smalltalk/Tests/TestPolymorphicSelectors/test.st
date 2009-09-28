" -[URLProtectionSpace port] returns an int, -[NSURL port] return an object "

"<< types = '{ port = i8@0:4; }' >>"
"<< types = '{ port = @8@0:4; }' >>"

NSObject subclass: SmalltalkTool [
	 run [
		| urlProtectionSpace url |
		urlProtectionSpace := NSURLProtectionSpace alloc initWithHost:'' port:42 protocol:'' realm:'' authenticationMethod:''.
		url := NSURL alloc initWithString: 'http://www.etoileos.com:12345'.

		ETTranscript show: urlProtectionSpace port;
			cr;
			show: url port. 
	 ]
]
