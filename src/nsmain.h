// -*- tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-

/*
 * Suika 2
 * Copyright (C) 2001-2022, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changed]
 *  - 2020/06/11 Created.
 */

#ifndef SUIKA_NSMAIN_H
#define SUIKA_NSMAIN_H

#import <Cocoa/Cocoa.h>
#import "suika.h"

// キーコード
enum {
    KC_SPACE = 49,
    KC_RETURN = 36,
    KC_UP = 126,
    KC_DOWN = 125,
    KC_C = 8,
};

// ビュー
@interface SuikaView : NSOpenGLView <NSWindowDelegate, NSApplicationDelegate>
{
}
- (void)timerFired:(NSTimer *)timer;
@end

#endif
