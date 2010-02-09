
NSObject subclass: ICantDecide [
  iCantDecide: arr [
    arr size
  ]
]

NSObject subclass: SmalltalkTool [
  run [
    ETTranscript show: 'This needs to compile.'; cr.
  ]
]
