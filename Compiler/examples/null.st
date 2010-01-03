NSObject subclass: SmalltalkTool
[
	run: aBlock [
		aBlock log.
		'Evaluating block' log.
		aBlock value log.
	]
	run [
		self run:[ 'foo' log. ].
	]
]
