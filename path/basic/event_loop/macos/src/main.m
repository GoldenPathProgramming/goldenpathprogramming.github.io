#import <Cocoa/Cocoa.h>
#import <Carbon/Carbon.h>
#include <stdio.h>

static bool quit = false;
static NSWindow *window;
static NSApplication *app;

enum { OSXUserEvent_WindowClose, OSXUserEvent_WindowResize, OSXUserEvent_LostFocus, };

@interface Delegate : NSObject<NSApplicationDelegate, NSWindowDelegate> @end
@implementation Delegate {
    bool live_resizing;
}
-(NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication*)sender {
    NSEvent *event = [NSEvent otherEventWithType:NSEventTypeApplicationDefined location:(NSPoint){0,0} modifierFlags:0 timestamp:[[NSProcessInfo processInfo] systemUptime] windowNumber:[window windowNumber] context:nil subtype:OSXUserEvent_WindowClose data1:0 data2:0];
    [app postEvent:event atStart:true];
    return NSTerminateCancel;
}
-(void)windowWillClose:(NSNotification *)notification {
    NSEvent *event = [NSEvent otherEventWithType:NSEventTypeApplicationDefined location:(NSPoint){0,0} modifierFlags:0 timestamp:[[NSProcessInfo processInfo] systemUptime] windowNumber:[window windowNumber] context:nil subtype:OSXUserEvent_WindowClose data1:0 data2:0];
    [app postEvent:event atStart:true];
}
-(void)windowDidResignKey:(NSNotification*)notification {
    NSEvent *event = [NSEvent otherEventWithType:NSEventTypeApplicationDefined location:(NSPoint){0,0} modifierFlags:0 timestamp:[[NSProcessInfo processInfo] systemUptime] windowNumber:[window windowNumber] context:nil subtype:OSXUserEvent_LostFocus data1:0 data2:0];
    [app postEvent:event atStart:false];
}
-(void)windowDidResize:(NSNotification *)notification {
    if (live_resizing) return;
    [self SendResizeEvent];
}
-(void)windowWillStartLiveResize:(NSNotification *)notification {
    live_resizing = true;
}
-(void)windowDidEndLiveResize:(NSNotification *)notification {
    live_resizing = false;
    [self SendResizeEvent];
}
-(void)SendResizeEvent {
    NSSize size = [[window contentView] convertRectToBacking:[[window contentView] bounds]].size;
    NSEvent *event = [NSEvent otherEventWithType:NSEventTypeApplicationDefined location:(NSPoint){0,0} modifierFlags:0 timestamp:[[NSProcessInfo processInfo] systemUptime] windowNumber:[window windowNumber] context:nil subtype:OSXUserEvent_WindowResize data1:size.width data2:size.height];
    [app postEvent:event atStart:false];
}
-(void)mouseEntered:(NSEvent *)event {
    printf("Mouse entered content view tracking area\n");
}
-(void)mouseExited:(NSEvent *)event {
    printf("Mouse exited content view tracking area\n");
}
-(void)mouseMoved:(NSEvent *)event {
    NSPoint p = [[window contentView] convertPointToBacking:[event locationInWindow]];
    printf("Mouse moved inside content view tracking area: %.1f %.1f\n", p.x, p.y);
}
@end

const char *KeycodeStr (uint8_t code);

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

    NSView *content_view = [window contentView];
    NSTrackingArea *tracking_area = [[NSTrackingArea alloc] initWithRect:content_view.bounds options:NSTrackingMouseEnteredAndExited | NSTrackingMouseMoved | NSTrackingActiveAlways | NSTrackingInVisibleRect | NSTrackingEnabledDuringMouseDrag owner:[window delegate] userInfo:nil];
    [content_view addTrackingArea:tracking_area];

    if ([app respondsToSelector:@selector(activate)]) [(id)app activate];
    else [app activateIgnoringOtherApps:true];

    while (!quit) {
        @autoreleasepool {
            for (;;) {
                NSEvent *e = [app nextEventMatchingMask:NSEventMaskAny untilDate:[NSDate distantPast] inMode:NSDefaultRunLoopMode dequeue:YES];
                if (!e) break;

                bool pass_event_along = true;
                bool pressed = true; // Used to group together key press/release events into the same code
                switch ([e type]) {
                    case NSEventTypeLeftMouseUp:
                    case NSEventTypeRightMouseUp:
                    case NSEventTypeOtherMouseUp:
                        pressed = false;
                    case NSEventTypeLeftMouseDown:
                    case NSEventTypeRightMouseDown:
                    case NSEventTypeOtherMouseDown: {
                        NSPoint ep = [[window contentView] convertPointToBacking:[e locationInWindow]];
                        static const char *mouse_button_name[5] = {
                            "Left", "Right", "Middle", "X1", "X2"
                        };
                        const char *button_name = "Unknown";
                        const int button_number = [e buttonNumber];
                        if (button_number < 5) button_name = mouse_button_name[button_number];
                        printf ("Mouse button %d(%s) %s at %f, %f\n", button_number, button_name, pressed ? "pressed" : "released", ep.x, ep.y);
                    } break;

                    case NSEventTypeMouseMoved:
                    case NSEventTypeLeftMouseDragged:
                    case NSEventTypeRightMouseDragged:
                    case NSEventTypeOtherMouseDragged: {
                        if (e.window == nil) {
                            NSRect frame = [window frame];
                            NSPoint p = [e locationInWindow];
                            p.x -= frame.origin.x;
                            p.y -= frame.origin.y;
                            p = [[window contentView] convertPointToBacking:p];
                            printf ("Mouse moved outside window: %.1f %.1f\n", p.x, p.y);
                        }
                        else {
                            NSPoint p = [[window contentView] convertPointToBacking:[e locationInWindow]];
                            printf("Mouse moved: %.1f %.1f\n", p.x, p.y);
                        }
                    } break;

                    case NSEventTypeScrollWheel: {
                        if ([e hasPreciseScrollingDeltas]) {
                            const NSTimeInterval time = [e timestamp];
                            const float dy = [e scrollingDeltaY];
                            printf ("Precise scroll %f @ %f (%f)\n", dy, time, [e deltaY]);
                        }
                        else {
                            const float dy = [e deltaY];
                            printf ("Simple scroll %f\n", dy);
                        }
                    } break;

                    case NSEventTypeKeyUp:
                        pressed = false;
                    case NSEventTypeKeyDown: {
                        pass_event_along = false;
                        if ([e isARepeat]) {
                            printf ("Key repeat %s\n", KeycodeStr([e keyCode]));
                            break;
                        }
                        printf ("Key %s %s\n", KeycodeStr((uint8_t)[e keyCode]), pressed ? "pressed" : "released");
                    } break;

                    case NSEventTypeFlagsChanged: {
                        typedef union {
                            struct {
                                uint8_t alpha_shift:1;
                                uint8_t shift:1;
                                uint8_t control:1;
                                uint8_t alternate:1;
                                uint8_t command:1;
                                uint8_t numeric_pad:1;
                                uint8_t help:1;
                                uint8_t function:1;
                            };
                            uint8_t mask;
                        } osx_event_modifiers_t;
                        static osx_event_modifiers_t mods_prev = {};
                        osx_event_modifiers_t mods = {.mask = ([e modifierFlags] & NSEventModifierFlagDeviceIndependentFlagsMask) >> 16};
                        if (mods.alpha_shift ^ mods_prev.alpha_shift) printf ("Key %s Alpha Shift\n", mods.alpha_shift ? "pressed" : "released");
                        if (mods.shift ^ mods_prev.shift) printf ("Key %s Shift\n", mods.shift ? "pressed" : "released");
                        if (mods.control ^ mods_prev.control) printf ("Key %s Ctrl\n", mods.control ? "pressed" : "released");
                        if (mods.alternate ^ mods_prev.alternate) printf ("Key %s Alt\n", mods.alternate ? "pressed" : "released");
                        if (mods.command ^ mods_prev.command) printf ("Key %s Command\n", mods.command ? "pressed" : "released");
                        if (mods.numeric_pad ^ mods_prev.numeric_pad) printf ("Key %s NumLock\n", mods.numeric_pad ? "pressed" : "released");
                        if (mods.help ^ mods_prev.help) printf ("Key %s Help\n", mods.help ? "pressed" : "released");
                        if (mods.function ^ mods_prev.function) printf ("Key %s Function\n", mods.function ? "pressed" : "released");
                        mods_prev = mods;
                    } break;

                    case NSEventTypeApplicationDefined: {
                        switch ([e subtype]) {
                            case OSXUserEvent_WindowClose: {
                                printf ("Window close\n");
                                quit = true;
                            } break;

                            case OSXUserEvent_WindowResize: printf ("Resize %d, %d\n", (int)[e data1], (int)[e data2]); break;

                            case OSXUserEvent_LostFocus: puts ("Focus Out"); break;

                            default: puts ("Unknown user event?"); break;
                        }
                    } break;
                    
                    default: break;
                }

                if (pass_event_along) [app sendEvent:e];
            }
            [app updateWindows];
            usleep(5000);
        }
    }

    return 0;
}

const char *KeycodeStr (uint8_t code) {
    static const char *keycode_str[256] = { [kVK_ANSI_A] = "A", [kVK_ANSI_B] = "B", [kVK_ANSI_C] = "C", [kVK_ANSI_D] = "D", [kVK_ANSI_E] = "E", [kVK_ANSI_F] = "F", [kVK_ANSI_G] = "G", [kVK_ANSI_H] = "H", [kVK_ANSI_I] = "I", [kVK_ANSI_J] = "J", [kVK_ANSI_K] = "K", [kVK_ANSI_L] = "L", [kVK_ANSI_M] = "M", [kVK_ANSI_N] = "N", [kVK_ANSI_O] = "O", [kVK_ANSI_P] = "P", [kVK_ANSI_Q] = "Q", [kVK_ANSI_R] = "R", [kVK_ANSI_S] = "S", [kVK_ANSI_T] = "T", [kVK_ANSI_U] = "U", [kVK_ANSI_V] = "V", [kVK_ANSI_W] = "W", [kVK_ANSI_X] = "X", [kVK_ANSI_Y] = "Y", [kVK_ANSI_Z] = "Z", [kVK_ANSI_0] = "0", [kVK_ANSI_1] = "1", [kVK_ANSI_2] = "2", [kVK_ANSI_3] = "3", [kVK_ANSI_4] = "4", [kVK_ANSI_5] = "5", [kVK_ANSI_6] = "6", [kVK_ANSI_7] = "7", [kVK_ANSI_8] = "8", [kVK_ANSI_9] = "9", [kVK_Return] = "Enter", [kVK_Space] = "Space", [kVK_Escape] = "Escape", [kVK_LeftArrow] = "Left", [kVK_RightArrow] = "Right", [kVK_DownArrow] = "Down", [kVK_UpArrow] = "Up", [kVK_Option] = "Left ALt", [kVK_RightOption] = "Right Alt", [kVK_Control] = "Left Control", [kVK_RightControl] = "Right Control", [kVK_Home] = "Home", [kVK_End] = "End", [kVK_PageUp] = "Page Up", [kVK_PageDown] = "Page Down", [kVK_F1] = "F1", [kVK_F2] = "F2", [kVK_F3] = "F3", [kVK_F4] = "F4", [kVK_F5] = "F5", [kVK_F6] = "F6", [kVK_F7] = "F7", [kVK_F8] = "F8", [kVK_F9] = "F9", [kVK_F10] = "F10", [kVK_F11] = "F11", [kVK_F12] = "F12", [kVK_Delete] = "Backspace", [kVK_ForwardDelete] = "Delete", [kVK_Shift] = "Shift", [kVK_Tab] = "Tab", [kVK_ANSI_Quote] = "'", [kVK_ANSI_Comma] = ",", [kVK_ANSI_Minus] = "-", [kVK_ANSI_Period] = ".", [kVK_ANSI_Slash] = "/", [kVK_ANSI_Semicolon] = ";", [kVK_ANSI_Equal] = "=", [kVK_ANSI_LeftBracket] = "[", [kVK_ANSI_RightBracket] = "]", [kVK_ANSI_Backslash] = "\\", [kVK_ANSI_Grave] = "`" };
    return keycode_str[code];
}