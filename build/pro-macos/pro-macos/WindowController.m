// -*- coding: utf-8; indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

/*
 * Suika2
 * Copyright (C) 2001-2024, Keiichi Tabata. All rights reserved.
 */

#import "WindowController.h"

@interface WindowController ()
{
}
@end

@implementation WindowController

- (void)windowDidLoad {
    [super windowDidLoad];
    
    self.window.delegate = (id<NSWindowDelegate>)self.contentViewController;
}

@end
