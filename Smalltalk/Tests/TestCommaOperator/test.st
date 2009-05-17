
NSObject subclass: SmalltalkTool [
	 run [
		" Tests that both strings and arrays support the comma operator, which
		  concatenates strings or joins arrays "
		ETTranscript show: 
			(({'H','el'.'lo,'},{' W'.'orld!'} inject: (NSMutableString string)
		                                       into: [ :aVal :collect | collect , aVal  ])) ; cr.
	 ]
]
