NSObject subclass: SmalltalkTool
[

	run 
	[ | task |
		NSApplication sharedApplication setDelegate: self.
		NSApplication sharedApplication run.
	]

	applicationDidFinishLaunching: notif 
	[
		'Hello world!' log.	
	]
]
