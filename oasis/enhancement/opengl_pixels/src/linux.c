// Compile with:
// gcc linux.c -lGL -lX11

#include <X11/Xatom.h>
#include <GL/glx.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

static struct {
    int32_t w, h;
} window = {.w = 640, .h = 480};

typedef struct {
    uint8_t r, g, b;
} pixel_t;

struct {
    int32_t width, height;
    pixel_t *pixels;
} pixel_buffer;

static void OnResize () {
    glViewport (0, 0, window.w, window.h);
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();

    pixel_buffer.width = window.w > 0 ? window.w : 1;
    pixel_buffer.height = window.h > 0 ? window.h : 1;
    pixel_buffer.pixels = (pixel_t*)realloc (pixel_buffer.pixels, pixel_buffer.width * pixel_buffer.height * sizeof (pixel_buffer.pixels[0]));
    assert (pixel_buffer.pixels);
    memset (pixel_buffer.pixels, 0, pixel_buffer.width * pixel_buffer.height * sizeof (pixel_buffer.pixels[0]));

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, pixel_buffer.width, pixel_buffer.height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
}

typedef union {
    struct { uint8_t major, minor; };
    uint16_t full;
} glversion_t;

bool GLHasExtension (const char *const extension) {
    const char *gl_extensions = (const char*)glGetString (GL_EXTENSIONS);

    const auto len = strlen (extension);
    for (;;) {
        gl_extensions = strstr (gl_extensions, extension);
        if (gl_extensions == NULL) return false;
        char next = *(gl_extensions + len);
        if (next == ' ' || next == '\0') return true;
        gl_extensions += len;
    }
    return false;
}

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

    XSetWindowAttributes attributes = {
        .background_pixel = 0x403a4d,
        .colormap = XCreateColormap (display, root_window, visual->visual, AllocNone),
        .event_mask = StructureNotifyMask | KeyPressMask | KeyReleaseMask | FocusChangeMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask,
    };
    const auto xwindow = XCreateWindow(display, root_window, 0, 0, 640, 480, 0, visual->depth, 0, visual->visual, CWBackPixel | CWColormap | CWEventMask, &attributes);

    #define WINDOW_TITLE "Golden Path"
    XChangeProperty (display, xwindow, XInternAtom (display, "_NET_WM_NAME", False), XInternAtom (display, "UTF8_STRING", False), 8, 0, (const unsigned char*)WINDOW_TITLE, sizeof (WINDOW_TITLE)-1);

    Atom WM_DELETE_WINDOW = XInternAtom (display, "WM_DELETE_WINDOW", False);
    if (WM_DELETE_WINDOW != None)
        { const auto result = XSetWMProtocols (display, xwindow, &WM_DELETE_WINDOW, 1); assert (result); }

    XMapWindow (display, xwindow);
    XFlush (display);

    typedef GLXContext (*glXCreateContextAttribsARB_t) (Display *dpy, GLXFBConfig config, GLXContext share_context, Bool direct, const int *attrib_list);
    const auto glXCreateContextAttribsARB = (glXCreateContextAttribsARB_t)glXGetProcAddress ((const GLubyte*)"glXCreateContextAttribsARB"); assert (glXCreateContextAttribsARB);

    const int context_attributes[] = {
        GLX_RENDER_TYPE, GLX_RGBA_TYPE,
        GLX_CONTEXT_MAJOR_VERSION_ARB, 2,
        GLX_CONTEXT_MINOR_VERSION_ARB, 1,
        None
    };

    const auto gl_context = glXCreateContextAttribsARB (display, fb_config, 0, True, context_attributes); assert(gl_context);

    { const auto result = glXMakeCurrent (display, xwindow, gl_context); assert (result); }

    const char *gl_version = (const char*)glGetString (GL_VERSION);
    printf ("OpenGL version: %s\n", gl_version);

    glversion_t glversion = {.major = gl_version[0] - '0', .minor = gl_version[2] - '0'};
    bool glversion_compatibility = true;
    if (glversion.full >= (glversion_t){.major=3,.minor=2}.full) { // Verion 3.2 onward have core and compatibility profiles. Core removes deprecated functionality.
        int context_profile = 0;
        glGetIntegerv (GL_CONTEXT_PROFILE_MASK, &context_profile);
        if (!(context_profile & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT)) glversion_compatibility = false;
    }
    else if (glversion.full == (glversion_t){.major=3,.minor=1}.full) { // Version 3.1 removed deprecated functionality, but did not have profiles yet.
        if (!GLHasExtension("GL_ARB_compatibility")) glversion_compatibility = false;
    }

    if (glversion.full < (glversion_t){.major = 2, .minor = 1}.full || (glversion.full > (glversion_t){.major = 2, .minor = 1}.full && !glversion_compatibility)) {
        puts ("OpenGL context 2.1, or >2.1 with compatibility profile required.");
        return 1;
    }

    glEnable (GL_TEXTURE_2D);

    GLuint tex;
    glGenTextures (1, &tex);
    glBindTexture (GL_TEXTURE_2D, tex);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    OnResize ();
    glClearColor (0, 0, 0, 0);

    bool quit = false;
    while (!quit) {
        while (XPending (display)) {
            XEvent e;
            XNextEvent (display, &e);
            switch (e.type) {
                case DestroyNotify: {
                    quit = true;
                } break;

                case ClientMessage: {
                    const auto c = &e.xclient;
                    if ((Atom)c->data.l[0] == WM_DELETE_WINDOW) {
                        quit = true;
                    }
                } break;

                case ConfigureNotify: {
                    const auto c = &e.xconfigure;
                    if (c->width != window.w || c->height != window.h) {
                        window.w = c->width;
                        window.h = c->height;
                        OnResize ();
                    }
                } break;

                default: break;
            }
        }

        glClear (GL_COLOR_BUFFER_BIT);

        static uint8_t r = 0;
        static int8_t rsign = 1;
        r += rsign;
        if (r == 255) rsign = -1;
        else if (r == 0) rsign = 1;

        for (int y = 0; y < pixel_buffer.height; ++y) {
            uint8_t g = y * 255 / pixel_buffer.height;
            for (int x = 0; x < pixel_buffer.width; ++x) {
                uint8_t b = x * 255 / pixel_buffer.width;

                const auto p = &pixel_buffer.pixels[x + y * pixel_buffer.width];
                p->r = r;
                p->g = g;
                p->b = b;
            }
        }

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	    glTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, pixel_buffer.width, pixel_buffer.height, GL_RGB, GL_UNSIGNED_BYTE, pixel_buffer.pixels);

        glBegin (GL_TRIANGLE_STRIP);
            glTexCoord2i (0, 1);
            glVertex3f (-1,  1, 0);
            glTexCoord2i (0, 0);
            glVertex3f (-1, -1, 0);
            glTexCoord2i (1, 1);
            glVertex3f ( 1, 1, 0);
            glTexCoord2i (1, 0);
            glVertex3f ( 1, -1, 0);
        glEnd ();

        glXSwapBuffers (display, xwindow);

        usleep (1000);
    }

    return 0;
}
