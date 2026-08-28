#define GL_SILENCE_DEPRECATION
#include <OpenGL/glu.h>
#import <Cocoa/Cocoa.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>

bool GLHasExtension (const char *const haystack, const char *const needle) {
    const char *e = haystack;
    const auto len = strlen (needle);
    for (;;) {
        e = strstr (e, needle);
        if (e == NULL) return false;
        char next = *(e + len);
        if (next == ' ' || next == '\0') return true;
        e += len;
    }
}

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

    const char *gl_extensions = (const char*)glGetString (GL_EXTENSIONS);
    printf ("\nGL extensions: %s\n\n", gl_extensions);

    const auto vertex_shader = glCreateShader (GL_VERTEX_SHADER);
    printf ("glCreateShader called: %u\n", vertex_shader);

    const auto gldll = dlopen ("/System/Library/Frameworks/OpenGL.framework/OpenGL", RTLD_LAZY);
    if (gldll) puts ("OpenGL dynamic library loaded");
    else {
        puts ("Failed to open OpenGL dynamic library");
        return 1;
    }

    if (GLHasExtension (gl_extensions, "GL_ARB_vertex_buffer_object")) {
        do {
            typedef void (*glGenBuffersARB_t) (GLsizei n, GLuint *buffers);

            const auto glGenBuffersARB = (glGenBuffersARB_t)dlsym (gldll, "glGenBuffersARB");
            if (glGenBuffersARB == NULL) {
                puts ("Failed to retrieve glGenBuffersARB function pointer");
                break;
            }

            GLuint buffer = 0;
            glGenBuffersARB (1, &buffer);
            printf ("glGenBuffersARB found and called: %d\n", buffer);
        } while (false);
    }
    else puts ("Extension not found: GL_ARB_vertex_buffer_object");

    return 0;
}
