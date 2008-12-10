
ETTranscript extend [
	 +puts: string [
	     self show: string.
	     self cr.
	 ]
]

NSObject subclass: SmalltalkTool [
	 run [
	     ETTranscript puts: 'class method invocation'.
	 ]
]
