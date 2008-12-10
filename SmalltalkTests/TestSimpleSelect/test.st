
NSObject subclass: SmalltalkTool [
    run [
		ETTranscript show: ({'testThis'. 'doThat'} select: [ :x | x hasPrefix: 'test' ]) ; cr.
    ]
]
