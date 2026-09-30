#define GL_SILENCE_DEPRECATION
#import <Cocoa/Cocoa.h>
#include <OpenGL/glu.h>

static bool quit = false;
static NSOpenGLContext *gl_context;
static NSApplication *app;
static NSWindow *window;

@interface Delegate : NSObject<NSApplicationDelegate, NSWindowDelegate> @end
@implementation Delegate {
    bool live_resizing;
}
-(NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication*)sender {
    quit = true;
    return NSTerminateCancel;
}
-(void)windowWillClose:(NSNotification*)notification { quit = true; }
-(void)windowDidResize:(NSNotification *)notification {
    if (live_resizing) return;
    [self Resize];
}
-(void)windowWillStartLiveResize:(NSNotification *)notification {
	live_resizing = true;
}
-(void)windowDidEndLiveResize:(NSNotification *)notification {
    live_resizing = false;
    [self Resize];
}
-(void)Resize {
    NSSize size = [[window contentView] convertRectToBacking:[[window contentView] bounds]].size;
    glViewport (0, 0, size.width, size.height);
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    [gl_context update];
}
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
    
    Delegate *delegate = [Delegate new];
    [app setDelegate:delegate];
    [window setDelegate:delegate];
    
    [window makeKeyAndOrderFront:window];

    NSOpenGLPixelFormatAttribute gl_attributes[] = {
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
    NSOpenGLPixelFormat *pixel_format = [[NSOpenGLPixelFormat alloc] initWithAttributes:gl_attributes];
    gl_context = [[NSOpenGLContext alloc] initWithFormat:pixel_format shareContext:nil];
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wdeprecated-declarations"
    [gl_context setView:[window contentView]];
    #pragma clang diagnostic pop
    [gl_context makeCurrentContext];

    printf ("GL context version: %s\n", glGetString (GL_VERSION));

    if (@available(macOS 14.0, *)) [(id)app activate];
    else [app activateIgnoringOtherApps:true];

    [delegate Resize];

    glClearColor (0, 0, 0, 0);

    while (!quit) {
        @autoreleasepool {
            for (;;) {
                NSEvent *e = [app nextEventMatchingMask:NSEventMaskAny untilDate:[NSDate distantPast] inMode:NSDefaultRunLoopMode dequeue:YES];
                if (!e) break;
                [app sendEvent:e];
            }
            [app updateWindows];

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
    }

    return 0;
}