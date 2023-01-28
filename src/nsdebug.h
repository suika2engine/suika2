// -*- tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-

/*
 * Suika 2
 * Copyright (C) 2001-2022, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changed]
 *  - 2020/06/11 Created.
 */

#ifndef SUIKA_NSDEBUG_H
#define SUIKA_NSDEBUG_H

#import <Cocoa/Cocoa.h>
#import "suika.h"

@interface DebugWindowController : NSWindowController <NSWindowDelegate, NSTableViewDataSource, NSTableViewDelegate>
- (void)doubleClickTableView:(id)nid;
@end

BOOL initDebugWindow(void);
BOOL getStartupPosition(void);

#endif
