
NSObject subclass: SmalltalkTool [
	 run [
	     | numbers animals space |
	     numbers := {'two'. 'three'. 'four'}.
	     animals := {'tigers'. 'ducks'}.
	     space := ' '.
	     numbers do: [ :number |
	     	     animals do: [ :animal |
		     	     ETTranscript show: number.
		     	     ETTranscript show: space.
		     	     ETTranscript show: animal.
			     ETTranscript cr.
		     ].
	     ].
	 ]
]
