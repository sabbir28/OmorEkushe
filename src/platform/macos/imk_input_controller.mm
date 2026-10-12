#if defined(__APPLE__)
#import "platform/macos/imk_input_controller.h"
#include "core/phonetic_engine.h"
#include "core/app_state.h"
static NSString* WStringToNSString(const std::wstring& str) {
    if (str.empty()) return @"";
    return [[NSString alloc] initWithBytes:str.data()
                                    length:str.size() * sizeof(wchar_t)
                                  encoding:NSUTF32StringEncoding];
}

@implementation OmorEkusheInputController {
    BOOL _phoneticActive;
}

- (id)initWithServer:(IMKServer *)server delegate:(id)delegate client:(id)client {
    self = [super initWithServer:server delegate:delegate client:client];
    if (self) {
        _phoneticActive = YES;
    }
    return self;
}

- (void)togglePhoneticMode {
    _phoneticActive = !_phoneticActive;
    bijoy::core::g_phoneticEngine.Reset();
}

- (void)commitComposition:(id)sender {
    bijoy::core::g_phoneticEngine.Reset();
    [sender setMarkedText:@"" selectionRange:NSMakeRange(0, 0) replacementRange:NSMakeRange(NSNotFound, NSNotFound)];
}

- (BOOL)handleEvent:(NSEvent *)event client:(id)sender {
    if ([event type] != NSEventTypeKeyDown) {
        return NO;
    }

    // F12 KeyCode is 111 on macOS keyboards
    if ([event keyCode] == 111) {
        [self togglePhoneticMode];
        return YES;
    }

    // Command or Control modifiers pass through
    NSUInteger modifiers = [event modifierFlags];
    if ((modifiers & NSEventModifierFlagCommand) || (modifiers & NSEventModifierFlagControl)) {
        [self commitComposition:sender];
        return NO;
    }

    if (!_phoneticActive) {
        return NO;
    }

    // Backspace: KeyCode 51
    if ([event keyCode] == 51) {
        uintptr_t clientHandle = (uintptr_t)(__bridge void*)sender;
        if (bijoy::core::g_phoneticEngine.ProcessBackspace(clientHandle)) {
            const std::wstring& lastOutput = bijoy::core::g_phoneticEngine.GetLastOutput();
            NSString* nsStr = WStringToNSString(lastOutput);
            [sender setMarkedText:nsStr selectionRange:NSMakeRange(nsStr.length, 0) replacementRange:NSMakeRange(NSNotFound, NSNotFound)];
            return YES;
        }
        return NO;
    }

    // Space, Return, Tab commit current composition
    if ([event keyCode] == 49 || [event keyCode] == 36 || [event keyCode] == 48) {
        const std::wstring& lastOutput = bijoy::core::g_phoneticEngine.GetLastOutput();
        if (!lastOutput.empty()) {
            NSString* nsStr = WStringToNSString(lastOutput);
            [sender insertText:nsStr replacementRange:NSMakeRange(NSNotFound, NSNotFound)];
            bijoy::core::g_phoneticEngine.Reset();
        }
        return NO; // Let Space/Return pass through to insert the delimiter
    }

    NSString* chars = [event characters];
    if (chars.length == 0) {
        return NO;
    }

    unichar ch = [chars characterAtIndex:0];
    if (ch >= 0x20 && ch <= 0x7E) {
        uintptr_t clientHandle = (uintptr_t)(__bridge void*)sender;
        if (bijoy::core::g_phoneticEngine.ProcessCharacter(static_cast<char>(ch), clientHandle)) {
            const std::wstring& lastOutput = bijoy::core::g_phoneticEngine.GetLastOutput();
            NSString* nsStr = WStringToNSString(lastOutput);
            [sender setMarkedText:nsStr selectionRange:NSMakeRange(nsStr.length, 0) replacementRange:NSMakeRange(NSNotFound, NSNotFound)];
            return YES;
        }
    }

    return NO;
}

@end
#endif

