#import <Cocoa/Cocoa.h>

static bool quit = false;

@interface Delegate : NSObject<NSApplicationDelegate, NSWindowDelegate> @end
@implementation Delegate
-(NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication*)sender {
	quit = true;
	return NSTerminateCancel;
}
-(void)windowWillClose:(NSNotification *)notification {
	quit = true;
}
@end

int main () {
    NSApplication *app = [NSApplication sharedApplication];
    [app setActivationPolicy:NSApplicationActivationPolicyRegular];

    NSMenu *menu_bar = [NSMenu new];
    NSMenuItem *menu_item_app = [NSMenuItem new];
    [menu_bar addItem:menu_item_app];
    [app setMainMenu:menu_bar];

    NSMenu *app_menu = [NSMenu new];
    [app_menu addItem:[[NSMenuItem alloc] initWithTitle:[@"Quit " stringByAppendingString:[[NSProcessInfo processInfo] processName]] action:@selector(terminate:) keyEquivalent:@"q"]];
    [menu_item_app setSubmenu:app_menu];

    NSWindow *window = [[NSWindow alloc] initWithContentRect:NSMakeRect(0,0,640,480) styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable backing:NSBackingStoreBuffered defer:YES];
    [window setReleasedWhenClosed:NO];
    [window setTitle:@"Golden Path"];
    [window setFrameAutosaveName:[window title]];
    [window makeKeyAndOrderFront:window];
    
    Delegate *delegate = [Delegate new];
    [app setDelegate:delegate];
    [window setDelegate:delegate];

    if (@available(macOS 14.0, *)) [(id)app activate];
    else [app activateIgnoringOtherApps:true];

    while (!quit) {
        @autoreleasepool {
            for (;;) {
                NSEvent *e = [app nextEventMatchingMask:NSEventMaskAny untilDate:[NSDate distantPast] inMode:NSDefaultRunLoopMode dequeue:YES];
                if (!e) break;
                [app sendEvent:e];
            }
            [app updateWindows];
        }
        usleep (5000);
    }

    return 0;
}