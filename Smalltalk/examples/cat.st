NSObject extend [
	wibble [
		'Yes!' log.
	]
]
NSObject subclass: SmalltalkTool [
	run [
		'Did categories work?' log.
		(NSObject new) wibble.
	]
]
