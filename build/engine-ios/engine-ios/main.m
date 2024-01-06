/* -*- coding: utf-8; tab-width: 4; indent-tabs-mode: nil; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2024, Keiichi Tabata. All rights reserved.
 */

@import UIKit;

#import "AppDelegate.h"

int main(int argc, char * argv[]) {
    NSString * appDelegateClassName;
    @autoreleasepool {
        // Setup code that might create autoreleased objects goes here.
        appDelegateClassName = NSStringFromClass([AppDelegate class]);
    }
    setlocale(LC_NUMERIC, "C");
    return UIApplicationMain(argc, argv, nil, appDelegateClassName);
}
