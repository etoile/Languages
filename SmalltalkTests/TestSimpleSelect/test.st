
NSObject subclass: SmalltalkTool [
    run [
    	"Strange Heisenbug? When cascading this show message with a cr message (as usual), it works."
	ETTranscript show: ({'testThis'. 'doThat'} select: [ :x | x hasPrefix: 'test' ]).
    ]
]
