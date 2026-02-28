#define GL_SILENCE_DEPRECATION
#import <Cocoa/Cocoa.h>
#include <OpenGL/glu.h>

int main () {
    const id window = [[NSWindow alloc] initWithContentRect:NSMakeRect(0,0,640,480) styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable backing:NSBackingStoreBuffered defer:YES];
    
    NSOpenGLPixelFormatAttribute glAttributes[] = {
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersionLegacy,
        // NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
        // NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion4_1Core,
        0,
    };
    NSOpenGLPixelFormat *pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:glAttributes];
    const NSOpenGLContext *gl_context = [[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:nil];
    [gl_context makeCurrentContext];

    printf ("GL context version: %s\n", glGetString (GL_VERSION));

    return 0;
}