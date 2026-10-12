#if defined(__APPLE__)
#import <Cocoa/Cocoa.h>
#include "core/phonetic_engine.h"
#include "core/app_state.h"

@interface OmorEkusheStatusItem : NSObject
@property (strong, nonatomic) NSStatusItem *statusItem;
@end

@implementation OmorEkusheStatusItem

- (instancetype)init {
    self = [super init];
    if (self) {
        self.statusItem = [[NSStatusBar systemStatusBar] statusItemWithLength:NSVariableStatusItemLength];
        self.statusItem.button.title = @"অমর একুশে";
        
        NSMenu *menu = [[NSMenu alloc] init];
        [menu addItemWithTitle:@"Toggle Phonetic (F12)" action:@selector(togglePhonetic:) keyEquivalent:@""];
        [menu addItem:[NSMenuItem separatorItem]];
        [menu addItemWithTitle:@"About Omor Ekushe" action:@selector(aboutApp:) keyEquivalent:@""];
        [menu addItem:[NSMenuItem separatorItem]];
        [menu addItemWithTitle:@"Quit" action:@selector(quitApp:) keyEquivalent:@"q"];
        
        self.statusItem.menu = menu;
    }
    return self;
}

- (void)togglePhonetic:(id)sender {
    bijoy::core::g_phoneticEngine.Reset();
}

- (void)aboutApp:(id)sender {
    NSAlert *alert = [[NSAlert alloc] init];
    alert.messageText = @"Omor Ekushe (অমর একুশে)";
    alert.informativeText = @"Native Bengali Keyboard Engine for macOS & Linux.\nSupports Avro Phonetic & Dynamic XML Layouts.";
    [alert addButtonWithTitle:@"OK"];
    [alert runModal];
}

- (void)quitApp:(id)sender {
    [NSApp terminate:nil];
}

@end

static OmorEkusheStatusItem *g_statusItem = nil;

namespace bijoy::platform::macos {

void CreateStatusBarItem() {
    if (!g_statusItem) {
        g_statusItem = [[OmorEkusheStatusItem alloc] init];
    }
}

} // namespace bijoy::platform::macos
#endif

