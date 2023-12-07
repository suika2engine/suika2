// -*- tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-

/*
 * Suika2
 * Copyright (C) 2001-2023, Keiichi Tabata. All rights reserved.
 */

/*
 * [Changed]
 *  - 2020/06/11 Created.
 *  - 2023/12/06 Refactored.
 */

#ifndef SUIKA_NSMAIN_H
#define SUIKA_NSMAIN_H

//
// GameView
//
@interface GameView : NSOpenGLView
@property AVPlayer *player;
@property AVPlayerLayer *playerLayer;
@property BOOL isMoviePlaying;
@property FILE *logFp;
- (void)cleanup;
- (void)timerFired:(NSTimer *)timer;
- (void)onPlayEnd:(NSNotification *)notification;
@end

//
// NSWindowController
//
#if !defined(USE_DEBUGGER)
@interface GameWindowController : NSWindowController <NSApplicationDelegate>
@property (strong) IBOutlet GameView *view;
@end
extern GameWindowController *windowController;
#else
@interface DebugWindowController : NSWindowController <NSApplicationDelegate>
@property (strong) IBOutlet GameView *view;
@end
extern DebugWindowController *windowController;
#endif

#endif
