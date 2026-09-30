#define GL_SILENCE_DEPRECATION
#import <Cocoa/Cocoa.h>
#include <OpenGL/glu.h>

int main () {
    NSWindow *window = [[NSWindow alloc] initWithContentRect:NSMakeRect(0,0,640,480) styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable backing:NSBackingStoreBuffered defer:YES];
    
    NSOpenGLPixelFormatAttribute gl_attributes[] = {
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersionLegacy,
        // NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
        // NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion4_1Core,
        0,
    };
    NSOpenGLPixelFormat *pixel_format = [[NSOpenGLPixelFormat alloc] initWithAttributes:gl_attributes];
    NSOpenGLContext *gl_context = [[NSOpenGLContext alloc] initWithFormat:pixel_format shareContext:nil];
    [gl_context makeCurrentContext];

    printf ("GL context version: %s\n", glGetString (GL_VERSION));

    return 0;
}