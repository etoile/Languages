"
 Tests mutually recursive class dependencies. (The List class needs to
 know the SuffixList class to create suffices, but the suffix class is
 derived from the list class.)

 The code is modeled after NSArray and NSString-like class clusters for
 extra realism.
"
NSObject subclass: List [
  suffixOfSize: n [
    ^ SuffixList new initWithSource: self start: (self theSize - n)
  ]
  print [
    ETTranscript show: '---'; cr.
    0 to: (self theSize - 1) do: [ :x |
      ETTranscript show: x; show: ': '; show: (self at: x); cr.
    ]
  ]
]

List subclass: SuffixList [
  | fSrc fStart |
  initWithSource: src start: start [
    fSrc := src.
    fStart := start
  ]
  at: i [ ^ fSrc at: fStart + i ]
  theSize [ ^ fSrc theSize - fStart ]
]

List subclass: ActualList [
  | fArray |
  initWithArray: arr [ fArray := arr ]
  at: i [ ^ fArray objectAtIndex: i ]
  theSize [ ^ fArray count ]
]

NSObject subclass: SmalltalkTool [
  run [
    | l |
    l := ActualList new initWithArray: { 'a'. 'b'. 'c'. 'd'. 'e' }.
    l print.
    (l suffixOfSize: 3) print.
  ]
]
