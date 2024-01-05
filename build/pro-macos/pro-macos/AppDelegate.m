// -*- coding: utf-8; indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

/*
 * Suika2
 * Copyright (C) 2001-2024, Keiichi Tabata. All rights reserved.
 */

#import "AppDelegate.h"

@interface AppDelegate ()
{
    BOOL _loaded;
}
@end

@implementation AppDelegate

- (void)application:(NSApplication *)application openURLs:(NSArray<NSURL *> *)urls {
    if (!_loaded) {
        NSString *base = [urls[0].path stringByDeletingLastPathComponent];
        NSFileManager *fileManager = [NSFileManager defaultManager];
        [fileManager changeCurrentDirectoryPath:base];
        _loaded = YES;
    }
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
}

- (BOOL)applicationSupportsSecureRestorableState:(NSApplication *)app {
    return YES;
}

@end
