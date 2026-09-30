#include <AppKit/NSWindow.h>
#import <Cocoa/Cocoa.h>
#include <stdbool.h>
#include <stdio.h>

enum { kVK_ANSI_B = 0x0B, kVK_Return = 0x24, kVK_Escape = 0x35 };

static bool quit = false;
static NSWindow *window;
static NSApplication *app;

static bool IsFullscreen () {
    return [NSApp currentSystemPresentationOptions] & NSApplicationPresentationFullScreen;
}

static void ToggleFullscreen () {
    [window toggleFullScreen:nil];
}

static bool IsBorderlessWindow () {
    return [window styleMask] == NSWindowStyleMaskBorderless;
}

static void ToggleBorderlessWindow () {
    static struct {
        NSWindowStyleMask style_mask;
        NSRect frame;
        NSApplicationPresentationOptions presentation_options;
    } window_data;

    if (IsBorderlessWindow()) {
        [app setPresentationOptions:window_data.presentation_options];
        [window setFrame: window_data.frame display: TRUE];
        [window setStyleMask: window_data.style_mask];
    }
    else {
        window_data.style_mask = [window styleMask];
        window_data.frame = [window convertRectToScreen:[[window contentView] frame]];
        window_data.presentation_options = [app presentationOptions];

        [app setPresentationOptions: NSApplicationPresentationHideMenuBar | NSApplicationPresentationHideDock];
        [window setStyleMask: NSWindowStyleMaskBorderless];
        [window setFrame: [[window screen] frame] display: TRUE];
    }
}

@interface Delegate : NSObject<NSApplicationDelegate, NSWindowDelegate> @end
@implementation Delegate
-(NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication*)sender {
    quit = true;
    return NSTerminateCancel;
}
-(void)windowWillClose:(NSNotification *)notification { quit = true; }
@end

int main () {
    app = [NSApplication sharedApplication];
    [app setActivationPolicy:NSApplicationActivationPolicyRegular];

    NSMenu *menu_bar = [NSMenu new];
    NSMenuItem *menu_item_app = [NSMenuItem new];
    [menu_bar addItem:menu_item_app];
    [app setMainMenu:menu_bar];

    NSMenu *app_menu = [NSMenu new];
    [app_menu addItem:[[NSMenuItem alloc] initWithTitle:[@"Quit " stringByAppendingString:[[NSProcessInfo processInfo] processName]] action:@selector(terminate:) keyEquivalent:@"q"]];
    [menu_item_app setSubmenu:app_menu];

    window = [[NSWindow alloc] initWithContentRect:NSMakeRect(0,0,640,480) styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable backing:NSBackingStoreBuffered defer:YES];
    [window setReleasedWhenClosed:NO];
    [window setTitle:@"Golden Path"];
    [window setFrameAutosaveName:[window title]];
    [window makeKeyAndOrderFront:window];
    [window setAcceptsMouseMovedEvents:YES];
    
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

                bool pass_event_along = true;
                switch ([e type]) {
                    case NSEventTypeKeyDown: {
                        pass_event_along = false;
                        switch ([e keyCode]) {
                            case kVK_Return: ToggleFullscreen (); break;
                            case kVK_ANSI_B: ToggleBorderlessWindow(); break;
                            case kVK_Escape: quit = true; break;
                            default: break;
                        }
                    } break;
                    
                    default: break;
                }

                if (pass_event_along) [app sendEvent:e];
            }

            [app updateWindows];
            usleep (5000);
        }
    }

    return 0;
}
