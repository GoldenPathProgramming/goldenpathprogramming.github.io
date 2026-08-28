#include <GL/glx.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef union {
    struct { uint8_t major, minor; };
    uint16_t full;
} glversion_t;

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
    const auto display = XOpenDisplay (NULL);
    assert (display);

    const auto root_window = DefaultRootWindow(display);
    const auto screen = DefaultScreen (display);

    int gl_attributes [] = {
        GLX_RGBA,
        GLX_DOUBLEBUFFER,
        GLX_RED_SIZE, 8, GLX_BLUE_SIZE, 8, GLX_GREEN_SIZE, 8, GLX_DEPTH_SIZE, 24,
        0
    };

    const auto visual = glXChooseVisual (display, screen, gl_attributes); assert (visual);
    const auto gl_context = glXCreateContext (display, visual, 0, true); assert (gl_context);

    XSetWindowAttributes attributes = {
        .background_pixel = 0x403a4d,
        .colormap = XCreateColormap (display, root_window, visual->visual, AllocNone),
        .event_mask = StructureNotifyMask | KeyPressMask | KeyReleaseMask | FocusChangeMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask,
    };
    const auto xwindow = XCreateWindow(display, root_window, 0, 0, 800, 600, 0, visual->depth, 0, visual->visual, CWBackPixel | CWColormap | CWEventMask, &attributes);

    { const auto result = glXMakeCurrent (display, xwindow, gl_context); assert (result); }

    const char *gl_version = (const char*)glGetString (GL_VERSION);
    printf ("OpenGL version: %s\n", gl_version);

    const char *gl_extensions = (const char*)glGetString (GL_EXTENSIONS);
    printf ("\nGL extensions: %s\n\n", gl_extensions);

    const char *glx_extensions = glXQueryExtensionsString (display,0);
    printf ("\nGLX extensions: %s\n\n", glx_extensions);

    glversion_t glversion = {.major = gl_version[0] - '0', .minor = gl_version[2] - '0'};
    bool glversion_compatibility = true;
    if (glversion.full >= (glversion_t){.major=3,.minor=2}.full) { // Verion 3.2 onward have core and compatibility profiles. Core removes deprecated functionality.
        int context_profile = 0;
        glGetIntegerv (GL_CONTEXT_PROFILE_MASK, &context_profile);
        if (!(context_profile & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT)) glversion_compatibility = false;
    }
    else if (glversion.full == (glversion_t){.major=3,.minor=1}.full) { // Version 3.1 removed deprecated functionality, but did not have profiles yet.
        if (!GLHasExtension(gl_extensions, "GL_ARB_compatibility")) glversion_compatibility = false;
    }

    while (glversion.full >= (glversion_t){.major = 2, .minor = 0}.full && glversion_compatibility) {
        typedef GLuint (*glCreateShader_t) (GLenum shaderType);
        
        const auto glCreateShader = (glCreateShader_t)glXGetProcAddress ((const GLubyte*)"glCreateShader");
        if (glCreateShader == NULL) {
            puts ("Failed to retrieve glCreateShader function pointer");
            break;
        }

        const auto vertex_shader = glCreateShader (GL_VERTEX_SHADER);
        printf ("glCreateShader found and called: %u\n", vertex_shader);
    break;}

    if (GLHasExtension (gl_extensions, "GL_ARB_vertex_buffer_object")) {
        do {
            typedef void (*glGenBuffersARB_t) (GLsizei n, GLuint *buffers);

            const auto glGenBuffersARB = (glGenBuffersARB_t)glXGetProcAddress ((const GLubyte*)"glGenBuffersARB");
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

    if (GLHasExtension (glx_extensions, "GLX_EXT_swap_control_tear")) {
        do {
            typedef void (*glXSwapIntervalEXT_t)(Display *dpy, GLXDrawable drawable, int interval);

            const auto glXSwapIntervalEXT = (glXSwapIntervalEXT_t)glXGetProcAddress((const GLubyte*)"glXSwapIntervalEXT");
            if (glXSwapIntervalEXT == NULL) {
                puts ("Failed to retrieve glXSwapIntervalEXT function pointer");
                break;
            }
            glXSwapIntervalEXT (display, xwindow, -1);
            puts ("glXSwapIntervalEXT found and called");
        } while (false);
    }
    else puts ("Extension not found: GLX_EXT_swap_control_tear");

    return 0;
}
