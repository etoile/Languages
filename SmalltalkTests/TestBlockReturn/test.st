
NSObject subclass: SmalltalkTool [
	 createCounter [
	     | count |
	     count := 0.
	     ^ [ count := count + 1 ]
	 ]
	 
	 log: item [
	     ETTranscript show: item.
	     ETTranscript cr.
	 ]
	 
	 run [
	     | a b |
	     a := self createCounter.
	     b := self createCounter.
	     self log: a value.
	     self log: a value.
	     self log: a value.
	     self log: b value.
	     self log: b value.
	     self log: a value.
	     self log: b value.
	 ]
]
