NSObject subclass: SmalltalkTool [
	run [
		self performSelector:'wibble'.
	]
	wibble [
		'wibble called' log.
	]
]
