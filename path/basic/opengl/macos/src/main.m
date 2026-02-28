#define GL_SILENCE_DEPRECATION
#import <Cocoa/Cocoa.h>
#include <OpenGL/glu.h>

static bool quit = false;
static id window;
enum {OSXUserEvent_WindowResize};
static bool live_resizing = false;
static NSOpenGLContext *gl_context;

#define WINDOW_CONTENT_SIZE [[window contentView] convertRectToBacking:[[window contentView] bounds]].size

static void OnResize () {
    const NSSize size = WINDOW_CONTENT_SIZE;
    glViewport (0, 0, size.width, size.height);
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    [gl_context update];
}

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
-(void)windowDidResize:(NSNotification *)notification;
-(void)windowWillStartLiveResize:(NSNotification *)notification;
-(void)windowDidEndLiveResize:(NSNotification *)notification;
@end
@implementation WindowDelegate
-(void)windowWillClose:(NSNotification *)notification {
    quit = true;
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

    NSOpenGLPixelFormatAttribute glAttributes[] = {
        NSOpenGLPFAColorSize, 24,
        NSOpenGLPFAAlphaSize, 8,
        NSOpenGLPFAClosestPolicy,
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFAAccelerated,
        NSOpenGLPFANoRecovery,
        NSOpenGLPFADepthSize, 24,
        NSOpenGLPFAStencilSize, 8,
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersionLegacy,
        0,
    };
    NSOpenGLPixelFormat *pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:glAttributes];
    gl_context = [[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:nil];
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wdeprecated-declarations"
    [gl_context setView:[window contentView]];
    #pragma clang diagnostic pop
    [gl_context makeCurrentContext];

    printf ("GL context version: %s\n", glGetString (GL_VERSION));

    [NSApp activate];

    OnResize ();
    glClearColor (0, 0, 0, 0);

    while (!quit) {
        NSEvent *e = [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:[NSDate distantPast] inMode:NSDefaultRunLoopMode dequeue:YES];
        if (e) {
            if (e.type == NSEventTypeApplicationDefined) {
                if (e.subtype == OSXUserEvent_WindowResize) {
                    OnResize();
                }
            }
            [NSApp sendEvent:e];
        }
        [NSApp updateWindows];

        glClear (GL_COLOR_BUFFER_BIT);

        glBegin (GL_TRIANGLES);
            glColor3f (1, 0, 0);
            glVertex3f (-1, -1, 0);
            glColor3f (0, 1, 0);
            glVertex3f (1, -1, 0);
            glColor3f (0, 0, 1);
            glVertex3f (0, 1, 0);
        glEnd ();

        [gl_context flushBuffer];
    }

    return 0;
}