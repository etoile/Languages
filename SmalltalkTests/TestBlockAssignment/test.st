
"This class stores a block."
NSObject subclass: FilteredStringPrinter [
	 | myBlock |
	 
	 withBlock: aBlock [
	     "This is a setter, not an initialiser."
	     myBlock := aBlock.
	     ETTranscript show: 'The aBlock object is a '; show: myBlock className; cr.
	     myBlock value: 'the secret string in the withBlock: method'.
		 ^self.
	 ]
	 
	 doSomethingWith: str [
	     "This method will print str when the block evaluates to true."
	     myBlock value: str.
	 ]
]


NSObject subclass: SmalltalkTool [
	 run [
	     | p |
	     
	     p := FilteredStringPrinter new withBlock: [ :x |
	          ETTranscript show: 'block called with '; show: x; cr.
	     ].
	     
	     p doSomethingWith: 'the first string';
	       doSomethingWith: 'the second string'.
	     
	     ETTranscript show: 'done'; cr.
	 ]
]
