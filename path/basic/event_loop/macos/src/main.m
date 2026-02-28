#import <Cocoa/Cocoa.h>
#include <stdio.h>

// Keycodes taken from HIToolbox/Events.h (part of Carbon, the old API for porting OS8/9 apps to OSX)
enum { kVK_ANSI_A = 0x00, kVK_ANSI_S = 0x01, kVK_ANSI_D = 0x02, kVK_ANSI_F = 0x03, kVK_ANSI_H = 0x04, kVK_ANSI_G = 0x05, kVK_ANSI_Z = 0x06, kVK_ANSI_X = 0x07, kVK_ANSI_C = 0x08, kVK_ANSI_V = 0x09, kVK_ANSI_B = 0x0B, kVK_ANSI_Q = 0x0C, kVK_ANSI_W = 0x0D, kVK_ANSI_E = 0x0E, kVK_ANSI_R = 0x0F, kVK_ANSI_Y = 0x10, kVK_ANSI_T = 0x11, kVK_ANSI_1 = 0x12, kVK_ANSI_2 = 0x13, kVK_ANSI_3 = 0x14, kVK_ANSI_4 = 0x15, kVK_ANSI_6 = 0x16, kVK_ANSI_5 = 0x17, kVK_ANSI_Equal = 0x18, kVK_ANSI_9 = 0x19, kVK_ANSI_7 = 0x1A, kVK_ANSI_Minus = 0x1B, kVK_ANSI_8 = 0x1C, kVK_ANSI_0 = 0x1D, kVK_ANSI_RightBracket = 0x1E, kVK_ANSI_O = 0x1F, kVK_ANSI_U = 0x20, kVK_ANSI_LeftBracket = 0x21, kVK_ANSI_I = 0x22, kVK_ANSI_P = 0x23, kVK_ANSI_L = 0x25, kVK_ANSI_J = 0x26, kVK_ANSI_Quote = 0x27, kVK_ANSI_K = 0x28, kVK_ANSI_Semicolon = 0x29, kVK_ANSI_Backslash = 0x2A, kVK_ANSI_Comma = 0x2B, kVK_ANSI_Slash = 0x2C, kVK_ANSI_N = 0x2D, kVK_ANSI_M = 0x2E, kVK_ANSI_Period = 0x2F, kVK_ANSI_Grave = 0x32, kVK_ANSI_KeypadDecimal = 0x41, kVK_ANSI_KeypadMultiply = 0x43, kVK_ANSI_KeypadPlus = 0x45, kVK_ANSI_KeypadClear = 0x47, kVK_ANSI_KeypadDivide = 0x4B, kVK_ANSI_KeypadEnter = 0x4C, kVK_ANSI_KeypadMinus = 0x4E, kVK_ANSI_KeypadEquals = 0x51, kVK_ANSI_Keypad0 = 0x52, kVK_ANSI_Keypad1 = 0x53, kVK_ANSI_Keypad2 = 0x54, kVK_ANSI_Keypad3 = 0x55, kVK_ANSI_Keypad4 = 0x56, kVK_ANSI_Keypad5 = 0x57, kVK_ANSI_Keypad6 = 0x58, kVK_ANSI_Keypad7 = 0x59, kVK_ANSI_Keypad8 = 0x5B, kVK_ANSI_Keypad9 = 0x5C };
/* keycodes for keys that are independent of keyboard layout*/
enum { kVK_Return = 0x24, kVK_Tab = 0x30, kVK_Space = 0x31, kVK_Delete = 0x33, kVK_Escape = 0x35, kVK_Command = 0x37, kVK_Shift = 0x38, kVK_CapsLock = 0x39, kVK_Option = 0x3A, kVK_Control = 0x3B, kVK_RightShift = 0x3C, kVK_RightOption = 0x3D, kVK_RightControl = 0x3E, kVK_Function = 0x3F, kVK_F17 = 0x40, kVK_VolumeUp = 0x48, kVK_VolumeDown = 0x49, kVK_Mute = 0x4A, kVK_F18 = 0x4F, kVK_F19 = 0x50, kVK_F20 = 0x5A, kVK_F5 = 0x60, kVK_F6 = 0x61, kVK_F7 = 0x62, kVK_F3 = 0x63, kVK_F8 = 0x64, kVK_F9 = 0x65, kVK_F11 = 0x67, kVK_F13 = 0x69, kVK_F16 = 0x6A, kVK_F14 = 0x6B, kVK_F10 = 0x6D, kVK_F12 = 0x6F, kVK_F15 = 0x71, kVK_Help = 0x72, kVK_Home = 0x73, kVK_PageUp = 0x74, kVK_ForwardDelete = 0x75, kVK_F4 = 0x76, kVK_End = 0x77, kVK_F2 = 0x78, kVK_PageDown = 0x79, kVK_F1 = 0x7A, kVK_LeftArrow = 0x7B, kVK_RightArrow = 0x7C, kVK_DownArrow = 0x7D, kVK_UpArrow = 0x7E };
/* ISO keyboards only*/
enum { kVK_ISO_Section = 0x0A };
/* JIS keyboards only*/
enum { kVK_JIS_Yen = 0x5D, kVK_JIS_Underscore = 0x5E, kVK_JIS_KeypadComma = 0x5F, kVK_JIS_Eisu = 0x66, kVK_JIS_Kana = 0x68 };

static bool quit = false;
static id window;
static bool live_resizing = false;

enum { OSXUserEvent_WindowClose, OSXUserEvent_WindowResize, OSXUserEvent_LostFocus, OSXUserEvent_EnterFullscreen, OSXUserEvent_ExitFullscreen, };

@interface AppDelegate : NSObject<NSApplicationDelegate>
-(NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication*)sender;
@end
@implementation AppDelegate
-(NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication*)sender {
	NSEvent *event = [NSEvent otherEventWithType:NSEventTypeApplicationDefined location:(NSPoint){0,0} modifierFlags:0 timestamp:[[NSProcessInfo processInfo] systemUptime] windowNumber:[window windowNumber] context:nil subtype:OSXUserEvent_WindowClose data1:0 data2:0];
	[NSApp postEvent:event atStart:true];
	return NSTerminateCancel;
}
@end

#define WINDOW_CONTENT_SIZE [[window contentView] convertRectToBacking:[[window contentView] bounds]].size

@interface WindowDelegate : NSObject<NSWindowDelegate>
-(void)windowWillClose:(NSNotification*)notification;
-(void)windowDidResignKey:(NSNotification*)notification;
-(void)windowDidResize:(NSNotification *)notification;
-(void)windowWillStartLiveResize:(NSNotification *)notification;
-(void)windowDidEndLiveResize:(NSNotification *)notification;
@end
@implementation WindowDelegate
-(void)windowWillClose:(NSNotification *)notification {
	NSEvent *event = [NSEvent otherEventWithType:NSEventTypeApplicationDefined location:(NSPoint){0,0} modifierFlags:0 timestamp:[[NSProcessInfo processInfo] systemUptime] windowNumber:[window windowNumber] context:nil subtype:OSXUserEvent_WindowClose data1:0 data2:0];
	[NSApp postEvent:event atStart:true];
}
-(void)windowDidResignKey:(NSNotification*)notification {
	NSEvent *event = [NSEvent otherEventWithType:NSEventTypeApplicationDefined location:(NSPoint){0,0} modifierFlags:0 timestamp:[[NSProcessInfo processInfo] systemUptime] windowNumber:[window windowNumber] context:nil subtype:OSXUserEvent_LostFocus data1:0 data2:0];
	[NSApp postEvent:event atStart:false];
}
-(void)windowDidResize:(NSNotification *)notification {
	if (live_resizing) return;
	NSSize size = WINDOW_CONTENT_SIZE;
	NSEvent *event = [NSEvent otherEventWithType:NSEventTypeApplicationDefined location:(NSPoint){0,0} modifierFlags:0 timestamp:[[NSProcessInfo processInfo] systemUptime] windowNumber:[window windowNumber] context:nil subtype:OSXUserEvent_WindowResize data1:size.width data2:size.height];
	[NSApp postEvent:event atStart:false];
}
-(void)windowWillStartLiveResize:(NSNotification *)notification {
	live_resizing = true;
}
-(void)windowDidEndLiveResize:(NSNotification *)notification {
	live_resizing = false;
	NSSize size = WINDOW_CONTENT_SIZE;
	NSEvent *event = [NSEvent otherEventWithType:NSEventTypeApplicationDefined location:(NSPoint){0,0} modifierFlags:0 timestamp:[[NSProcessInfo processInfo] systemUptime] windowNumber:[window windowNumber] context:nil subtype:OSXUserEvent_WindowResize data1:size.width data2:size.height];
	[NSApp postEvent:event atStart:false];
}
@end

const char *KeycodeStr (uint8_t code);

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

    window = [[NSWindow alloc] initWithContentRect:NSMakeRect(0,0,640,480) styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable backing:NSBackingStoreBuffered defer:YES];
    [window setReleasedWhenClosed:NO];
    [window setTitle:@"Golden Path"];
    [window setFrameAutosaveName:[window title]];
    [window makeKeyAndOrderFront:window];
    
    [NSApp setDelegate:[AppDelegate new]];
    [window setDelegate:[WindowDelegate new]];

    [NSApp activate];

    while (!quit) {
        NSEvent *e = [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:[NSDate distantPast] inMode:NSDefaultRunLoopMode dequeue:YES];

        bool pass_event_along = true;
        bool pressed = true; // Used to group together key press/release events into the same code
        if (e) {
            switch ([e type]) {
                case NSEventTypeLeftMouseDragged:
                case NSEventTypeRightMouseDragged:
                case NSEventTypeOtherMouseDragged:
                case NSEventTypeMouseMoved: {
                    NSPoint ep = [[window contentView] convertPointToBacking:[e locationInWindow]];
                    printf ("Mouse moved: %f, %f\n", ep.x, ep.y);
                } break;

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

                case NSEventTypeKeyDown:
                    pressed = false;
                case NSEventTypeKeyUp: {
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
                    if (mods.command ^ mods_prev.command) printf ("Key %s Command\n", mods.alpha_shift ? "pressed" : "released");
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
                        case OSXUserEvent_EnterFullscreen: puts ("Fullscreen"); break;
                        case OSXUserEvent_ExitFullscreen: puts ("Exit fullscreen"); break;
                        default: puts ("Unknown user event?"); break;
                    }
                } break;
                
                case NSEventTypeMouseEntered: puts ("Mouse entered"); break;
                case NSEventTypeMouseExited: puts ("Mouse exited"); break;
                default: break;
            }

            if (pass_event_along) [NSApp sendEvent:e];
        }

        [NSApp updateWindows];
    }

    return 0;
}

const char *KeycodeStr (uint8_t code) {
    static const char *keycode_str[256] = { [kVK_ANSI_A] = "A", [kVK_ANSI_B] = "B", [kVK_ANSI_C] = "C", [kVK_ANSI_D] = "D", [kVK_ANSI_E] = "E", [kVK_ANSI_F] = "F", [kVK_ANSI_G] = "G", [kVK_ANSI_H] = "H", [kVK_ANSI_I] = "I", [kVK_ANSI_J] = "J", [kVK_ANSI_K] = "K", [kVK_ANSI_L] = "L", [kVK_ANSI_M] = "M", [kVK_ANSI_N] = "N", [kVK_ANSI_O] = "O", [kVK_ANSI_P] = "P", [kVK_ANSI_Q] = "Q", [kVK_ANSI_R] = "R", [kVK_ANSI_S] = "S", [kVK_ANSI_T] = "T", [kVK_ANSI_U] = "U", [kVK_ANSI_V] = "V", [kVK_ANSI_W] = "W", [kVK_ANSI_X] = "X", [kVK_ANSI_Y] = "Y", [kVK_ANSI_Z] = "Z", [kVK_ANSI_0] = "0", [kVK_ANSI_1] = "1", [kVK_ANSI_2] = "2", [kVK_ANSI_3] = "3", [kVK_ANSI_4] = "4", [kVK_ANSI_5] = "5", [kVK_ANSI_6] = "6", [kVK_ANSI_7] = "7", [kVK_ANSI_8] = "8", [kVK_ANSI_9] = "9", [kVK_Return] = "Enter", [kVK_Space] = "Space", [kVK_Escape] = "Escape", [kVK_LeftArrow] = "Left", [kVK_RightArrow] = "Right", [kVK_DownArrow] = "Down", [kVK_UpArrow] = "Up", [kVK_Option] = "Left ALt", [kVK_RightOption] = "Right Alt", [kVK_Control] = "Left Control", [kVK_RightControl] = "Right Control", [kVK_Home] = "Home", [kVK_End] = "End", [kVK_PageUp] = "Page Up", [kVK_PageDown] = "Page Down", [kVK_F1] = "F1", [kVK_F2] = "F2", [kVK_F3] = "F3", [kVK_F4] = "F4", [kVK_F5] = "F5", [kVK_F6] = "F6", [kVK_F7] = "F7", [kVK_F8] = "F8", [kVK_F9] = "F9", [kVK_F10] = "F10", [kVK_F11] = "F11", [kVK_F12] = "F12", [kVK_Delete] = "Backspace", [kVK_ForwardDelete] = "Delete", [kVK_Shift] = "Shift", [kVK_Tab] = "Tab", [kVK_ANSI_Quote] = "'", [kVK_ANSI_Comma] = ",", [kVK_ANSI_Minus] = "-", [kVK_ANSI_Period] = ".", [kVK_ANSI_Slash] = "/", [kVK_ANSI_Semicolon] = ";", [kVK_ANSI_Equal] = "=", [kVK_ANSI_LeftBracket] = "[", [kVK_ANSI_RightBracket] = "]", [kVK_ANSI_Backslash] = "\\", [kVK_ANSI_Grave] = "`" };
    return keycode_str[code];
}