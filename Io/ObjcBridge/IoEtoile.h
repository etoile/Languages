#ifndef __IO_ETOILE__
#define __IO_ETOILE__

#include <DistributedView/UKDistributedView.h>

/* Various protocol declarations similar to those found in IoObjcBridge.h
   It enables the use of informal protocols found in Etoile frameworks for
   GNUstep/Cocoa applications written in Io.
   We need this so the runtime will know the correct method signatures,
   otherwise applications would segfault at runtime on trying to use 
   'forward' slot for the unknown method. */

@protocol DistributedView

//UKDistributedView data source
-(int) numberOfItemsInDistributedView: (UKDistributedView*)distributedView;
-(NSPoint) distributedView: (UKDistributedView*)distributedView 
	positionForCell: (NSCell*)cell 
	atItemIndex: (int)row;
-(void)	distributedView: (UKDistributedView*)distributedView
	setPosition: (NSPoint)pos
	forItemIndex: (int)row;
-(void)	distributedView: (UKDistributedView*)distributedView
	setObjectValue: (id)val
	forItemIndex: (int)row;
-(NSString*) distributedView: (UKDistributedView*)distributedView 
	toolTipForItemAtIndex: (int)row;

// UKDistributedView drag n' drop
-(BOOL)	distributedView: (UKDistributedView*)dv 
	writeItems:(NSArray*)indexes
	toPasteboard: (NSPasteboard*)pboard;
-(NSDragOperation) distributedView: (UKDistributedView*)dv
	draggingSourceOperationMaskForLocal: (BOOL)isLocal;
-(NSDragOperation) distributedView: (UKDistributedView*)dv 
	validateDrop: (id <NSDraggingInfo>)info
	proposedItem: (int*)row;
-(BOOL) distributedView: (UKDistributedView*)dv 
	acceptDrop:(id <NSDraggingInfo>)info
	onItem:(int)row;
-(void)	distributedView: (UKDistributedView*)dv 
	dragEndedWithOperation: (NSDragOperation)operation;

//UKDistributed delegate
-(void) distributedView: (UKDistributedView*)distributedView 
	cellClickedAtItemIndex: (int)item;
-(void) distributedView: (UKDistributedView*)distributedView 
	cellDoubleClickedAtItemIndex: (int)item;
-(BOOL) distributedView: (UKDistributedView*)distributedView 
	shouldSelectItemIndex: (int)item;
-(void) distributedView: (UKDistributedView*)distributedView 
	didSelectItemIndex: (int)item;
-(int) distributedView: (UKDistributedView*)distributedView 
	itemIndexForString: (NSString*)str options: (unsigned)opts;
-(void) distributedViewDidStartCachingItems: (UKDistributedView*)distributedView;
-(void) distributedViewWillEndCachingItems: (UKDistributedView*)distributedView;

@end

@protocol Etoile <DistributedView>

@end

#endif
