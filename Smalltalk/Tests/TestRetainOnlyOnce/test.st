
"
  Problem: The log message in -retain appears very often,
  then a segfault occurs (try redirecting stdout to /dev/null
  for easier reproduction).
"

NSObject subclass: Car [
	retain [
		ETTranscript show: 'I retain only once.'; cr.
		super retain.
	]
	
	drive [
		ETTranscript show: 'Vroom, vroom!'; cr.
	]
]


NSObject subclass: SmalltalkTool [
	 run [
	 	 | car |
		 car := Car new.
		 car drive.
	 ]
]
