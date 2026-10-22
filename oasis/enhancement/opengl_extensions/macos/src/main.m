#define GL_SILENCE_DEPRECATION
#include <OpenGL/glu.h>
// #include <OpenGL/gl3.h>
#import <Cocoa/Cocoa.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>

bool GLHasExtensionPre30 (const char *const extension) {
    const char *gl_extensions = (const char*)glGetString (GL_EXTENSIONS);

    const size_t len = strlen (extension);
    for (;;) {
        gl_extensions = strstr (gl_extensions, extension);
        if (gl_extensions == NULL) return false;
        char next = *(gl_extensions + len);
        if (next == ' ' || next == '\0') return true;
        gl_extensions += len;
    }
    return false;
}

// bool GLHasExtension30Plus (const char *const extension) {
//     GLint extension_count; 
//     glGetIntegerv (GL_NUM_EXTENSIONS, &extension_count);

//     for (int i = 0; i < extension_count; ++i) {
//         const GLubyte *e = glGetStringi (GL_EXTENSIONS, i);
//         if (strcmp ((const char*)e, extension) == 0) return true;
//     }
//     return false;
// }

bool (*GLHasExtension) (const char *const extension) = GLHasExtensionPre30;

int main () {
    NSWindow *window = [[NSWindow alloc] initWithContentRect:NSMakeRect(0,0,640,480) styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable backing:NSBackingStoreBuffered defer:YES];
    
    NSOpenGLPixelFormatAttribute gl_attributes[] = {
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersionLegacy,
        // NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
        // NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion4_1Core,
        0,
    };
    NSOpenGLPixelFormat *pixel_format = [[NSOpenGLPixelFormat alloc] initWithAttributes:gl_attributes];
    const NSOpenGLContext *gl_context = [[NSOpenGLContext alloc] initWithFormat:pixel_format shareContext:nil];
    [gl_context makeCurrentContext];

    printf ("GL context version: %s\n", glGetString (GL_VERSION));

    const GLuint vertex_shader = glCreateShader (GL_VERTEX_SHADER);
    printf ("glCreateShader called: %u\n", vertex_shader);

    void *gldll = dlopen ("/System/Library/Frameworks/OpenGL.framework/OpenGL", RTLD_LAZY);
    if (gldll) puts ("OpenGL dynamic library loaded");
    else {
        puts ("Failed to open OpenGL dynamic library");
        return 1;
    }

    if (GLHasExtension ("GL_ARB_vertex_buffer_object")) {
        do {
            typedef void (*glGenBuffersARB_t) (GLsizei n, GLuint *buffers);

            const glGenBuffersARB_t glGenBuffersARB = (glGenBuffersARB_t)dlsym (gldll, "glGenBuffersARB");
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
