NSObject subclass: SmalltalkTool [
	run [
		(100 > 200) log.
		(200 > 100) log.
		(100 < 200) log.
		(200 < 100) log.
	]
]
