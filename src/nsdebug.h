// -*- tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-

/*
 * Suika 2
 * Copyright (C) 2001-2022, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changed]
 *  - 2020/06/11 Created.
 *  - 2023/12/05 Change to an editor.
 */

#ifndef SUIKA_NSDEBUG_H
#define SUIKA_NSDEBUG_H

#import <Cocoa/Cocoa.h>
#import "suika.h"

@interface DebugAppDelegate : NSObject
@end

@interface DebugWindowController : NSWindowController <NSWindowDelegate>
@end


BOOL initProject(void);
BOOL initDebugger(void);
void cleanupDebugger(void);

#endif
