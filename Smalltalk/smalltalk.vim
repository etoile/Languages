syntax clear
set nospell

" Block colouring, based on lisp.vim's rainbowparen stuff:
syn region smalltalkParen0           matchgroup=hlLevel0 start="`\=(" end=")" skip="|.\{-}|" contains=@smalltalkListCluster,smalltalkParen1
syn region smalltalkParen1 contained matchgroup=hlLevel1 start="`\=(" end=")" skip="|.\{-}|" contains=@smalltalkListCluster,smalltalkParen2
syn region smalltalkParen2 contained matchgroup=hlLevel2 start="`\=(" end=")" skip="|.\{-}|" contains=@smalltalkListCluster,smalltalkParen3
syn region smalltalkParen3 contained matchgroup=hlLevel3 start="`\=(" end=")" skip="|.\{-}|" contains=@smalltalkListCluster,smalltalkParen4
syn region smalltalkParen4 contained matchgroup=hlLevel4 start="`\=(" end=")" skip="|.\{-}|" contains=@smalltalkListCluster,smalltalkParen5
syn region smalltalkParen5 contained matchgroup=hlLevel5 start="`\=(" end=")" skip="|.\{-}|" contains=@smalltalkListCluster,smalltalkParen6
syn region smalltalkParen6 contained matchgroup=hlLevel6 start="`\=(" end=")" skip="|.\{-}|" contains=@smalltalkListCluster,smalltalkParen7
syn region smalltalkParen7 contained matchgroup=hlLevel7 start="`\=(" end=")" skip="|.\{-}|" contains=@smalltalkListCluster,smalltalkParen8
syn region smalltalkParen8 contained matchgroup=hlLevel8 start="`\=(" end=")" skip="|.\{-}|" contains=@smalltalkListCluster,smalltalkParen9
syn region smalltalkParen9 contained matchgroup=hlLevel9 start="`\=(" end=")" skip="|.\{-}|" contains=@smalltalkListCluster,smalltalkParen0

syn region smalltalkParen0           matchgroup=hlLevel0 start="\[" end="]" skip="|.\{-}|" contains=@smalltalkListCluster,smalltalkParen1
syn region smalltalkParen1 contained matchgroup=hlLevel1 start="\[" end="]" skip="|.\{-}|" contains=@smalltalkListCluster,smalltalkParen2
syn region smalltalkParen2 contained matchgroup=hlLevel2 start="\[" end="]" skip="|.\{-}|" contains=@smalltalkListCluster,smalltalkParen3
syn region smalltalkParen3 contained matchgroup=hlLevel3 start="\[" end="]" skip="|.\{-}|" contains=@smalltalkListCluster,smalltalkParen4
syn region smalltalkParen4 contained matchgroup=hlLevel4 start="\[" end="]" skip="|.\{-}|" contains=@smalltalkListCluster,smalltalkParen5
syn region smalltalkParen5 contained matchgroup=hlLevel5 start="\[" end="]" skip="|.\{-}|" contains=@smalltalkListCluster,smalltalkParen6
syn region smalltalkParen6 contained matchgroup=hlLevel6 start="\[" end="]" skip="|.\{-}|" contains=@smalltalkListCluster,smalltalkParen7
syn region smalltalkParen7 contained matchgroup=hlLevel7 start="\[" end="]" skip="|.\{-}|" contains=@smalltalkListCluster,smalltalkParen8
syn region smalltalkParen8 contained matchgroup=hlLevel8 start="\[" end="]" skip="|.\{-}|" contains=@smalltalkListCluster,smalltalkParen9
syn region smalltalkParen9 contained matchgroup=hlLevel9 start="\[" end="]" skip="|.\{-}|" contains=@smalltalkListCluster,smalltalkParen0

if &bg == "dark"
	hi def hlLevel0 ctermfg=red         guifg=red1
	hi def hlLevel1 ctermfg=yellow      guifg=orange1
	hi def hlLevel2 ctermfg=green       guifg=yellow1
	hi def hlLevel3 ctermfg=cyan        guifg=greenyellow
	hi def hlLevel4 ctermfg=magenta     guifg=green1
	hi def hlLevel5 ctermfg=red         guifg=springgreen1
	hi def hlLevel6 ctermfg=yellow      guifg=cyan1
	hi def hlLevel7 ctermfg=green       guifg=slateblue1
	hi def hlLevel8 ctermfg=cyan        guifg=magenta1
	hi def hlLevel9 ctermfg=magenta     guifg=purple1
else
	hi def hlLevel0 ctermfg=red         guifg=red3
	hi def hlLevel1 ctermfg=darkyellow  guifg=orangered3
	hi def hlLevel2 ctermfg=darkgreen   guifg=orange2
	hi def hlLevel3 ctermfg=blue        guifg=yellow3
	hi def hlLevel4 ctermfg=darkmagenta guifg=olivedrab4
	hi def hlLevel5 ctermfg=red         guifg=green4
	hi def hlLevel6 ctermfg=darkyellow  guifg=paleturquoise3
	hi def hlLevel7 ctermfg=darkgreen   guifg=deepskyblue4
	hi def hlLevel8 ctermfg=blue        guifg=darkslateblue
	hi def hlLevel9 ctermfg=darkmagenta guifg=darkviolet
endif

syn keyword stBuiltin self super true false nil Nil
syn keyword	stKeyword	class new not alloc init
syn match	stKeyword	"\^"
syn match	stKeyword	":="
syn match	stBuiltin	"\<extend\>"
syn match	stBuiltin	"\<subclass\>:"

syn match	stMethod	"\<ifTrue\>:"
syn match	stMethod	"\<whileTrue\>:"
syn match	stMethod	"\<ifFalse\>:"
" the block of local variables of a method
syn region stLocalVariables	start="^[ \t]*|" end="|"

" the Smalltalk comment
syn region stComment	start="\"" end="\""

" the Smalltalk strings and single characters
syn region stString	start='\'' skip="\\'" end='\''
syn match  stCharacter	"$."

syn case ignore
" the symols prefixed by a '#'
syn match  stSymbol	"\(#[a-zA-Z_][a-z0-9:_]*\)"


" the variables in a statement block for loops
syn match  stBlockVariable "\(:[ \t]*\<[a-z_][a-z0-9_]*\>[ \t]*\)\+|" contained

" some representations of numbers
syn match  stNumber	"\<\d\+\>"
syn match  stFloat	"\<\d\+\.\d\+\>"

syn case match

syn match  stNSClass	"\<NS[a-z_]*\>"

" a try to higlight paren mismatches
syn region stParen	transparent start='(' end=')' contains=ALLBUT,stParenError
syn match  stParenError	")"
syn region stBlock	transparent start='\[' end='\]' contains=ALLBUT,stBlockError
syn match  stBlockError	"\]"
syn region stSet	transparent start='{' end='}' contains=ALLBUT,stSetError
syn match  stSetError	"}"

hi link stParenError stError
hi link stSetError stError
hi link stBlockError stError

" synchronization for syntax analysis
syn sync minlines=50

" Define the default highlighting.
" For version 5.7 and earlier: only when not done already
" For version 5.8 and later: only when an item doesn't have highlighting yet
if version >= 508 || !exists("did_st_syntax_inits")
  if version < 508
    let did_st_syntax_inits = 1
    command -nargs=+ HiLink hi link <args>
  else
    command -nargs=+ HiLink hi def link <args>
  endif

  HiLink stBlockVariable	Identifier
  HiLink stBuiltin		Special
  HiLink stCharacter		Constant
  HiLink stComment		Comment
  HiLink stError		Error
  HiLink stFloat		Constant
  HiLink stKeyword		Statement
  HiLink stLocalVariables	Identifier
  HiLink stMethod		Statement
  HiLink stNSClass		Special
  HiLink stNumber		Constant 
  HiLink stString		Constant
  HiLink stSymbol		Constant

  delcommand HiLink
endif

syn cluster smalltalkListCluster contains=stKeyword,stMethod,stComment,stCharacter,stString,stNSClass,stFloat,stError,stLocalVariables,stBlockVariable,stNumber,stSymbol,stBuiltin

set ai

let b:current_syntax = "st"
