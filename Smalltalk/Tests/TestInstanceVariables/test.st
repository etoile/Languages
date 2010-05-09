
NSObject subclass: A[
	| a b c |
	init [
		a := 'a'.
		b := 'b'.
		c := 'c'.
		^self.
	]
]
A subclass: B [
	| d e |
	init [
		super init.
		d := 'd'.
		e := 'e'.
		^self.
	]
	log [
		ETTranscript show: a;
		show: b;
		show: c;
		show: d;
		show: e; cr.
	]
]
NSObject subclass: SmalltalkTool [
	run [
		B new log.
	]
]
