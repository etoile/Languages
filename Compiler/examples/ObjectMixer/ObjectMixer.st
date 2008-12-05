LKAST extend 
[
	properties
	[
		^ { 'displayName'. 'icon'. 'ASTType'. }.
	]

	ASTType [ ^ nil. ]

	code
	[
		^ self description.
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
		^ methods.
	]

]

LKMethod extend
[
	displayName [ ^ self methodName. ]

	methodName [ ^ signature selector. ]

	ASTType [ ^ 'Method'. ]

	childNodes [ ^ statements. ]
]


NSObject subclass: SmalltalkTool
[
	| objectMixerItem browserItem codeEditorItem |

	run 
	[ | task |
		ETApplication sharedApplication setDelegate: self.
		ETApplication sharedApplication run.
	]

	applicationDidFinishLaunching: notif 
	[
		browserItem := ETContainer alloc init layoutItem.
		browserItem container setDelegate: self.
		browserItem container setSource: browserItem.
		browserItem setRepresentedObject: (LKAST code).
		browserItem setLayout: (ETOutlineLayout layout).
		browserItem layout setDisplayedProperties: (self displayedPropertiesInBrowser).
		browserItem layout setDisplayName: 'Type' forProperty: 'ASTType'.
		browserItem setWidth: 700.
		browserItem setHeight: 200.

		codeEditorItem := ETLayoutItem textView.
		codeEditorItem view setDelegate: self.
		codeEditorItem setWidth: 700.
		codeEditorItem setHeight: 400.

		objectMixerItem := ETContainer alloc init layoutItem.
		objectMixerItem setName: 'Object Mixer (Smalltalk & cie)'.
		objectMixerItem setLayout: (ETStackLayout layout).
		objectMixerItem setWidth: 700.
		objectMixerItem setHeight: 600.
		objectMixerItem container setEnablesHitTest: 1.

		objectMixerItem addItem: browserItem.
		objectMixerItem addItem: codeEditorItem.

		ETLayoutItem windowGroup addItem: objectMixerItem.

		browserItem container release.
		codeEditorItem container release.
		objectMixerItem container release.

		objectMixerItem inspect: nil.
	]

	displayedPropertiesInBrowser [ ^ { 'displayName'. 'ASTType' }. ]

	containerSelectionDidChange: notif
	[
		| codeOfSelectedASTNodes |

		notif log.
		codeOfSelectedASTNodes := NSMutableString alloc initWithCapacity: 10000.
		browserItem selectedItemsInLayout foreach: [ :item |
			codeOfSelectedASTNodes appendString: (item representedObject code). ].
		codeEditorItem view setString: codeOfSelectedASTNodes.
		codeOfSelectedASTNodes release.
	]
]
