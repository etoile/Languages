
NSObject subclass: SmalltalkTool [
	 run [
	     | a count |
	     
	     count := 0.
	     a := [
	       count := count + 1.
	       ETTranscript show: 'Outer block called. ('; show: count;
	       		    show: ')'; cr.
	       [
		 ETTranscript show: 'Inner block called.'; cr.
	       ] copy.
	     ] copy.
	     
	     "Call outer"
	     a value.
	     "Call outer, then inner two times"
	     a value value; value.
	     "Call outer"
	     a value.
	 ]
]
