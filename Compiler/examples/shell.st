#!/usr/bin/env -S edlc -f
NSObject subclass: SmalltalkTool [
	run [
		'Smalltalk tool executed with these arguments:' log.
		NSProcessInfo processInfo arguments mappedCollection log.
	]
]
