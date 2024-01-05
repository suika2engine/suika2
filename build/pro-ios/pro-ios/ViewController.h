/* -*- coding: utf-8; indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2024, Keiichi Tabata. All rights reserved.
 */

@import UIKit;
@import AVFoundation;

#import "GameViewControllerProtocol.h"

@interface ViewController : UIViewController <UITextViewDelegate, GameViewControllerProtocol>

@property BOOL isControlPressed;
@property BOOL isShiftPressed;
@property BOOL isCommandPressed;

- (void)onScriptShiftEnter;
- (void)onScriptRangedChange;
- (void)onScriptChange;

@end
