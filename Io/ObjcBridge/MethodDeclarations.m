//
//  MethodDeclarations.m
//  Io
//
//  Created by Steve Dekorte on Mon Nov 10 2003.
//  Copyright (c) 2003 __MyCompanyName__. All rights reserved.
//

#import "MethodDeclarations.h"

@implementation MethodDeclarations

- (void)browser:(NSBrowser *)sender createRowsForColumn:(int)column inMatrix:(NSMatrix *)matrix{}
- (BOOL)browser:(NSBrowser *)sender isColumnValid:(int)column{ return 1; }
- (int)browser:(NSBrowser *)sender numberOfRowsInColumn:(int)column{ return 1; }
- (BOOL)browser:(NSBrowser *)sender selectCellWithString:(NSString *)title inColumn:(int)column{ return 1; }

- (BOOL)browser:(NSBrowser *)sender selectRow:(int)row inColumn:(int)column{ return 1;}

- (float)browser:(NSBrowser *)browser shouldSizeColumn:(int)columnIndex forUserResize:(BOOL)forUserResize toWidth:(float)suggestedWidth{ return 1; }
- (float)browser:(NSBrowser *)browser sizeToFitWidthOfColumn:(int)columnIndex{ return 1; }
- (NSString *)browser:(NSBrowser *)sender titleOfColumn:(int)column{ return @"";}
- (void)browser:(NSBrowser *)sender willDisplayCell:(id)cell atRow:(int)row column:(int)colum{}
- (void)browserColumnConfigurationDidChange:(NSNotification *)notification{}
- (void)browserDidScroll:(NSBrowser *)sender{}
- (void)browserWillScroll:(NSBrowser *)sender{}

@end
