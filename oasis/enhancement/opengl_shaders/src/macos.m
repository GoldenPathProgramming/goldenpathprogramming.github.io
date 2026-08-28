// Build with:
// clang main.m -std=gnu2y -framework Cocoa -framework OpenGL
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

    const auto vertex = glCreateShader (GL_VERTEX_SHADER);
    glShaderSource (vertex, 1, &(const char*){
R"(#version 120

varying vec4 color;

void main()
{
    vec4 scale = vec4(1, 0.25, 1, 1);
    gl_Position = gl_ModelViewProjectionMatrix * scale * gl_Vertex;
    color = gl_Color;
})"}, NULL);

    glCompileShader (vertex);
    int success = 0;
    glGetShaderiv (vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        char buf[512];
        glGetShaderInfoLog (vertex, sizeof (buf), NULL, buf);
        printf ("OpenGL vertex shader compilation error: %s\n", buf);
        return -1;
    }

    const auto fragment = glCreateShader (GL_FRAGMENT_SHADER);
    glShaderSource (fragment, 1, &(const char *){
R"(#version 120

varying vec4 color;

void main()
{
    vec4 color_scale = vec4 (1, 0, 1, 1);
    gl_FragColor = color * color_scale;
}
)"}, NULL);

    glCompileShader (fragment);
    glGetShaderiv (fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        char buf[512];
        glGetShaderInfoLog (fragment, sizeof (buf), NULL, buf);
        printf ("OpenGL fragment shader compilation error: %s\n", buf);
        return -1;
    }

    const auto program = glCreateProgram ();
    glAttachShader (program, vertex);
    glAttachShader (program, fragment);
    glLinkProgram (program);
    glGetShaderiv (program, GL_LINK_STATUS, &success);
    if (!success) {
        char buf[512];
        glGetProgramInfoLog (program, sizeof (buf), NULL, buf);
        printf ("OpenGL shader linking error: %s\n", buf);
        return -1;
    }

    glDeleteShader (vertex);
    glDeleteShader (fragment);
    glUseProgram (program);
    glValidateProgram (program);
    glGetProgramiv (program, GL_VALIDATE_STATUS, &success);
    if (!success) {
        char buf[512];
        glGetProgramInfoLog (program, 512, NULL, buf);
        printf ("OpenGL shader validation error [%s]", buf);
        return -1;
    }

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