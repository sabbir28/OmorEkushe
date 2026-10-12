#pragma once

#if defined(__APPLE__)
#import <Cocoa/Cocoa.h>
#import <InputMethodKit/InputMethodKit.h>

/// Native macOS Input Method Kit Controller
@interface OmorEkusheInputController : IMKInputController

- (BOOL)handleEvent:(NSEvent *)event client:(id)sender;
- (void)commitComposition:(id)sender;
- (void)togglePhoneticMode;

@end
#endif

