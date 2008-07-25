AST extend 
[
	isOrdered 
	[
		^ 0.
	]

	children
	[
		^ nil.
	]

	isEmpty
	[
		^ self children count == 0.
	]

	content
	[
		^ self children.
	]

	contentArray
	[
		^ self children.
	]

	code
	[
		^ self description.
	]

	conformsToProtocol: protocol
	[
		^ protocol isEqual: (ETProtocol protocolForName: 'ETCollection').
	]
]

Subclass extend
[
	displayName
	[
		^ (classname stringByAppendingString: ' ') stringByAppendingString: 'Class'.
	]

	children
	[
		^ methods.
	]
]

CategoryDef extend
[
	displayName
	[
		^ (classname stringByAppendingString: ' ') stringByAppendingString: 'Category'.
	]

	children
	[
		^ NSMutableArray arrayWithArray: methods.
	]

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
		browserItem setLayout: (ETOutlineLayout layout).
		browserItem container setSource: browserItem.
		browserItem setRepresentedObject: (AST code).
		browserItem setWidth: 700.
		browserItem setHeight: 200.
		browserItem container setDelegate: self.

		codeEditorItem := ETLayoutItem textView.
		codeEditorItem view setDelegate: self.
		codeEditorItem setWidth: 700.
		codeEditorItem setHeight: 400.

		objectMixerItem := ETContainer alloc init layoutItem.
		objectMixerItem setLayout: (ETStackLayout layout).
		objectMixerItem setWidth: 700.
		objectMixerItem setHeight: 600.
		objectMixerItem container setEnablesHitTest: 1.

		objectMixerItem addItem: browserItem.
		objectMixerItem addItem: codeEditorItem.

		ETLayoutItem windowGroup addItem: objectMixerItem.

		objectMixerItem inspect: nil.
	]

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
