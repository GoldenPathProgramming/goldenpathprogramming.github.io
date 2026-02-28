#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <GL/glx.h>
#include <assert.h>
#include <stdio.h>

typedef GLXContext (*glXCreateContextAttribsARB_t) (Display *dpy, GLXFBConfig config, GLXContext share_context, Bool direct, const int *attrib_list); glXCreateContextAttribsARB_t glXCreateContextAttribsARB;

int main () {
    const auto display = XOpenDisplay (NULL);
    assert (display);

    const auto root_window = DefaultRootWindow(display);
    const auto screen = DefaultScreen (display);

    const int fb_attributes[] = {
        GLX_X_RENDERABLE, True,
        GLX_DOUBLEBUFFER, True,
        GLX_RED_SIZE, 8, GLX_GREEN_SIZE, 8, GLX_BLUE_SIZE, 8, GLX_DEPTH_SIZE, 24,
        None
    };

    int fb_count;
    GLXFBConfig* fb_configs = glXChooseFBConfig(display, screen, fb_attributes, &fb_count); assert(fb_configs && fb_count > 0);
    GLXFBConfig fb_config = fb_configs[0];
    XFree (fb_configs);

    XVisualInfo* visual = glXGetVisualFromFBConfig(display, fb_config); assert(visual);

    auto gl_context_temp = glXCreateContext (display, visual, 0, true); assert (gl_context_temp);

    XSetWindowAttributes attributes = {
        .background_pixel = 0x403a4d,
        .colormap = XCreateColormap (display, root_window, visual->visual, AllocNone),
        .event_mask = StructureNotifyMask | KeyPressMask | KeyReleaseMask | FocusChangeMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask,
    };
    const auto xwindow = XCreateWindow(display, root_window, 0, 0, 640, 480, 0, visual->depth, 0, visual->visual, CWBackPixel | CWColormap | CWEventMask, &attributes);
    
    glXCreateContextAttribsARB = (glXCreateContextAttribsARB_t)glXGetProcAddress ((const GLubyte*)"glXCreateContextAttribsARB"); assert (glXCreateContextAttribsARB);

    const int context_attributes[] = {
        GLX_RENDER_TYPE, GLX_RGBA_TYPE,
        GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
        GLX_CONTEXT_MINOR_VERSION_ARB, 2,
        GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
        None
    };

    const auto gl_context = glXCreateContextAttribsARB (display, fb_config, 0, True, context_attributes); assert(gl_context);

    { const auto result = glXMakeCurrent (display, xwindow, gl_context); assert (result); }

    glXDestroyContext (display, gl_context_temp);

    printf ("OpenGL version: %s\n", (const char*)glGetString (GL_VERSION));

    return 0;
}
