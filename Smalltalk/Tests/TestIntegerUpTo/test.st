
NSObject subclass: SmalltalkTool [
  run [
    | print arr |
    print := [ :x | ETTranscript show: x; cr. ].
    arr := { 'a'. 'b'. 'c'. 'd' }.
    0 to: 10 do: print.
    1000000000 to: 1000000010 do: print.
  ]
]
