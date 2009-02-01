"
  This is a pretty hardcore test for closures I guess.
  The createCounter method returns a closure when it's
  called. This is done two times here. Each of the two
  closures should have its own enclosed count variable.
  When the returned blocks are evaluated multiple times,
  they are supposed to return increasing numbers.
"
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
