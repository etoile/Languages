LKAST extend 
[
	properties
	[
		^ { 'displayName'. 'icon'. 'ASTType'. }.
	]

	ASTType [ ^ nil. ]

	sourceCode
	[ | desc |

		desc := self description.
		desc == nil ifTrue: [ desc := '' ].
		^ desc.
	]

	isOrdered 
	[
		^ 0.
	]

	childNodes
	[
		^ nil.
	]

	isEmpty
	[
		^ self childNodes count == 0.
	]

	content
	[
		^ self childNodes.
	]

	contentArray
	[
		^ self childNodes.
	]

	conformsToProtocol: protocol
	[
		^ protocol isEqual: (ETProtocol protocolForName: 'ETCollection').
	]
]

LKSubclass extend
[
	displayName [	^ classname. ]

	ASTType [ ^ 'Class'. ]

	childNodes
	[
		^ methods.
	]
]

LKCategoryDef extend
[
	displayName [	^ classname. ]

	ASTType [ ^ 'Category'. ]

	childNodes
	[
		^ self methods.
	]

]

LKMethod extend
[
	displayName [ ^ self methodName. ]

	methodName [ ^ self signature selector. ]

	ASTType [ ^ 'Method'. ]

	childNodes [ ^ self statements. ]
]

LKModule extend
[
	displayName [ ^ self primitiveDescription. ]

	ASTType [ ^ 'Module'. ]

	childNodes [ ^ (self allClasses) arrayByAddingObjectsFromArray: (self allCategories). ]
]

NSObject subclass: SmalltalkTool
[
	| objectMixerItem browserItem codeEditorItem |

	run 
	[ | task |
		ETApplication sharedApplication setDelegate: self.
		ETApplication sharedApplication run.
	]

	"Returns an array of AST nodes usuallay LKModules"
	parseCode
	[ | parser scriptSource ast op result |

		op := NSOpenPanel openPanel.
		op setAllowsMultipleSelection: true.
		result := op runModalForTypes: { 'st'. }.

		ast := NSMutableArray new.
		op filenames foreach: [ :scriptPath | 
			parser := (LKCompiler compilerClassForFileExtension: 'st') parserClass new.
        		scriptSource := NSString stringWithContentsOfFile: scriptPath.
			ast addObject: (parser parseString: scriptSource). ].
		^ast.
	]

	rootNode
	[ | astArray |
	
		astArray := self parseCode.
		(astArray count == 1) ifTrue: [ ^ astArray firstObject. ] ifFalse: [ ^ astArray. ].
	]

	applicationDidFinishLaunching: notif 
	[ | itemFactory |
		
		itemFactory := ETLayoutItem class.
		browserItem := itemFactory itemGroupWithContainer.
		browserItem setDelegate: self.
		browserItem setSource: browserItem.
		browserItem setRepresentedObject: (self parseCode).
		browserItem setLayout: (ETOutlineLayout layout).
		browserItem layout setDisplayedProperties: (self displayedPropertiesInBrowser).
		browserItem layout setDisplayName: 'Type' forProperty: 'ASTType'.
		browserItem setWidth: 700.
		browserItem setHeight: 200.

		codeEditorItem := itemFactory textView.
		codeEditorItem view retain.
		codeEditorItem setWidth: 700.
		codeEditorItem setHeight: 400.

		objectMixerItem := itemFactory itemGroupWithContainer.
		objectMixerItem setName: 'Object Mixer (Smalltalk & cie)'.
		objectMixerItem setLayout: (ETStackLayout layout).
		objectMixerItem setWidth: 700.
		objectMixerItem setHeight: 600.
		objectMixerItem supervisorView setEnablesHitTest: 1.

		objectMixerItem addItem: browserItem.
		objectMixerItem addItem: codeEditorItem.
		itemFactory windowGroup addItem: objectMixerItem.
		"browserItem reloadAndUpdateLayout."
		objectMixerItem explore: nil.
		objectMixerItem inspect: nil.
	]

	displayedPropertiesInBrowser [ ^ { 'displayName'. 'ASTType' }. ]

	editorView [ ^ codeEditorItem view documentView. ] "FIXME: Fix -[ETLayoutItem+Factory textView] to really return a text view"

	itemGroupSelectionDidChange: notif
	[
		| codeOfSelectedASTNodes |

		notif log.
		codeOfSelectedASTNodes := NSMutableString alloc initWithCapacity: 10000.
		browserItem selectedItemsInLayout foreach: [ :item | 
			codeOfSelectedASTNodes appendString: (item representedObject sourceCode). ].
		self editorView log. 
self editorView setString: codeOfSelectedASTNodes.
		codeOfSelectedASTNodes log.
		codeOfSelectedASTNodes release.
	]
]

