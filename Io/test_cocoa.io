/* #io
   docCopyright("Yen-Ju Chen", 2006)
   docLicense("BSD revised") */

ObjcBridge autoLookupClassNamesOn // useless

/* Io */
"Test" println

/* NSDate */
Date := ObjcBridge classNamed("NSDate")

date := Date date
date description println

/* NSNumber */
N := ObjcBridge classNamed("NSNumber")
n := N numberWithInt:(200)
n println

/* NSString */
String := ObjcBridge classNamed("NSString")
string := String stringWithCString:("Test")
string println
/* Warning: string becomes io object automatically. 
   Therefore, it is not a NSString anymore */

/* Application */
NSApp := ObjcBridge classNamed("NSApplication") sharedApplication

frame := method(x, y, w, h,
        List clone append(Point clone set(x, y), Point clone set(w, h))
)

Window := ObjcBridge classNamed("NSWindow")
win := Window alloc initWithContentRect:styleMask:backing:defer:(frame(200, 500, 420, 150), 15, 2, 0)
win setTitle:("Io Window")

quitButton := ObjcBridge classNamed("NSButton") alloc initWithFrame:(frame(330, 10, 76, 25))
win contentView addSubview:(quitButton)
quitButton setBezelStyle:(4)
quitButton setTarget:(NSApp)
quitButton setAction:("terminate:")
quitButton setTitle:("Quit")

win display
win orderFrontRegardless

NSApp run
