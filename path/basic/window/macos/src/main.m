#import <Cocoa/Cocoa.h>

static bool quit = false;

@interface AppDelegate : NSObject<NSApplicationDelegate>
-(NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication*)sender;
@end
@implementation AppDelegate
-(NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication*)sender {
	quit = true;
	return NSTerminateCancel;
}
@end

@interface WindowDelegate : NSObject<NSWindowDelegate>
-(void)windowWillClose:(NSNotification*)notification;
@end
@implementation WindowDelegate
-(void)windowWillClose:(NSNotification *)notification {
	quit = true;
}
@end

int main () {
    id app = [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

    id menuBar = [NSMenu new];
    id menuItemApp = [NSMenuItem new];
    [menuBar addItem:menuItemApp];
    [NSApp setMainMenu:menuBar];

    id appMenu = [NSMenu new];
    [appMenu addItem:[[NSMenuItem alloc] initWithTitle:[@"Quit " stringByAppendingString:[[NSProcessInfo processInfo] processName]] action:@selector(terminate:) keyEquivalent:@"q"]];
    [menuItemApp setSubmenu:appMenu];

    id window = [[NSWindow alloc] initWithContentRect:NSMakeRect(0,0,640,480) styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable backing:NSBackingStoreBuffered defer:YES];
    [window setReleasedWhenClosed:NO];
    [window setTitle:@"Golden Path"];
    [window setFrameAutosaveName:[window title]];
    [window makeKeyAndOrderFront:window];
    
    [NSApp setDelegate:[AppDelegate new]];
    [window setDelegate:[WindowDelegate new]];

    [NSApp activate];

    while (!quit) {
        NSEvent *e = [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:[NSDate distantPast] inMode:NSDefaultRunLoopMode dequeue:YES];
        if (e) [NSApp sendEvent:e];
        [NSApp updateWindows];
    }

    return 0;
}