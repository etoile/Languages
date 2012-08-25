<< headers='("dispatch/dispatch.h")' >>

NSObject subclass: MapResults [
	| d lock |
	init [
		super init.
		lock := NSLock new.
		d := NSMutableDictionary new.
	]

	addResult: x forKey: k [
		| old |
		lock lock.
		old := d objectForKey: k.
		(nil == old) ifTrue: [ old := NSMutableArray new. d setObject: old forKey: k ].
		old addObject: x.
		lock unlock.
	]
	results [ ^d ]
]

NSObject subclass: MapReduce [
	| queue input |

	init [
		super init.
		queue := C dispatch_get_global_queue: { 0. 0 }.
	]

	setInput: someData [ input := someData ]

	map: mapBlock reduce: reduceblock [
		| cond lock count results mapResults reduceResults |
		cond := NSCondition new.
		results := MapResults new.
		reduceResults := MapResults new.
		lock := NSLock new.
		cond lock.
		count := input count.
		count = 0 ifTrue: [ ^nil ].
		input foreach: [ :i |
			C dispatch_async: { queue .
				[ | result |
					result := mapBlock value: i value: results.
					lock lock .
					count := count - 1 .
					(count == 0) ifTrue: [ cond lock. cond signal. cond unlock ] .
					lock unlock .
				]
			}
		] .
		cond wait.
		mapResults := results results.
		count := mapResults count.
		count = 0 ifTrue: [ ^nil ].
		mapResults allKeys foreach: [ :i |
			C dispatch_async: { queue .
				[ | result |
					result := reduceblock value: i value: (mapResults objectForKey: i) value: reduceResults.
					lock lock .
					count := count - 1 .
					(count == 0) ifTrue: [ cond lock. cond signal. cond unlock ] .
					lock unlock .
				]
			}
		] .
		cond wait.
		cond unlock.
		^reduceResults results.
	]

]

NSObject subclass: SmalltalkTool
[
	run [
		| mr fm r args |
		mr := MapReduce new.
		fm := NSFileManager new.
		mr setInput: (fm contentsOfDirectoryAtPath: '.' error: nil).
		r := mr   map: [ :x :results |
			(x pathExtension isEqualToString: 'st') ifTrue: [
				| text |
				text := NSString stringWithContentsOfFile: x .
				text := text componentsSeparatedByCharactersInSet: (NSCharacterSet alphanumericCharacterSet invertedSet).
				text foreach: [ :token |
					token = '' ifFalse: [
						results addResult: token forKey: x
					]
				]
			]
		]
		       reduce: [ :k :v :r |
		    " Parser bug doesn't let us have arguments and locals in the same block :-( "
			[ | counts c |
				counts := NSMutableDictionary new.
				v foreach: [ :i | 
					c := counts objectForKey: i.
					(nil == c) ifTrue: [ c := 0. ].
					c := c + 1.
					counts setObject: c forKey: i
				] .
				counts allKeys foreach: [ :token |
					r addResult: (ETKeyValuePair pairWithKey: k value: (counts objectForKey: token)) forKey: token
				]
			] value.
		].
		(r allKeys sortedArrayUsingSelector: #caseInsensitiveCompare: ) foreach: [ :token |
			ETTranscript show: token ; cr.
			(r objectForKey: token) foreach: [ :entry |
				ETTranscript show: '\t'; show: (entry key); show: ': ' ; show: (entry value) ; cr.
			]
		]
	]
]
