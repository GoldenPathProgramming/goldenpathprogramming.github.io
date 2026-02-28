#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <GL/glx.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

static struct {
    int16_t w, h;
} window = {.w = 640, .h = 480};

typedef union {
    struct { uint8_t minor, major; };
    uint16_t full;
} glversion_t;

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
    const auto xwindow = XCreateWindow(display, root_window, 0, 0, window.w, window.h, 0, visual->depth, 0, visual->visual, CWBackPixel | CWColormap | CWEventMask, &attributes);

    { const auto result = glXMakeCurrent (display, xwindow, gl_context); assert (result); }

    const char *gl_version_string = (const char *)glGetString (GL_VERSION); assert (gl_version_string); assert (strlen(gl_version_string) >= 3);
    printf ("OpenGL version string: %s\n", gl_version_string);
    const char major = gl_version_string[0]; assert (major >= '1' && major <= '4');
    const char minor = gl_version_string[2]; assert (major >= '0' && major <= '6');

    glversion_t glversion = {.major = major - '0', .minor = minor - '0'};
    printf ("GL major.minor version: %d.%d\n", glversion.major, glversion.minor);

    if (glversion.full >= (glversion_t){.major=3,.minor=2}.full) { // Verion 3.2 onward have core and compatibility profiles. Core removes deprecated functionality.
        int context_profile = 0;
        glGetIntegerv (GL_CONTEXT_PROFILE_MASK, &context_profile);
        if (context_profile & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT) printf ("Compatibility context\n");
        else if (context_profile & GL_CONTEXT_CORE_PROFILE_BIT) printf ("Core context\n");
        else printf ("Unable to retrieve GL_CONTEXT_PROFILE_MASK even though version is >= 3.2???\n");
    }
    else if (glversion.full == (glversion_t){.major=3,.minor=1}.full) { // Version 3.1 removed deprecated functionality, but did not have profiles yet.
        const char *extensions = (const char *)glGetString (GL_EXTENSIONS); assert (extensions);
        if (strstr (extensions, "GL_ARB_compatibility")) printf ("GL_ARB_compatibility is present\n");
        else printf ("GL_ARB_compatibility NOT present\n");
    }

    return 0;
}