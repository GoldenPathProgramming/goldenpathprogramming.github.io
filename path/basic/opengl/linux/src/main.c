#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <GL/glx.h>
#include <assert.h>
#include <stdio.h>

static struct {
    int16_t w, h;
} window = {.w = 640, .h = 480};

static void OnResize () {
    glViewport (0, 0, window.w, window.h);
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
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
    const auto xwindow = XCreateWindow(display, root_window, 0, 0, window.w, window.h, 0, visual->depth, 0, visual->visual, CWBackPixel | CWColormap | CWEventMask, &attributes);

    { const auto result = glXMakeCurrent (display, xwindow, gl_context); assert (result); }

    printf ("OpenGL version: %s\n", (const char*)glGetString (GL_VERSION));

    XMapWindow (display, xwindow);
    XFlush (display);

    #define WINDOW_TITLE "Golden Path"
    XChangeProperty (display, xwindow, XA_WM_NAME, XA_STRING, 8, 0, (const unsigned char*)WINDOW_TITLE, sizeof (WINDOW_TITLE)-1);

    Atom WM_DELETE_WINDOW = XInternAtom (display, "WM_DELETE_WINDOW", False);
    if (WM_DELETE_WINDOW != None)
        { const auto result = XSetWMProtocols (display, xwindow, &WM_DELETE_WINDOW, 1); assert (result); }
    
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
                    const auto c = (XClientMessageEvent*)&e;
                    if (WM_DELETE_WINDOW && (Atom)c->data.l[0] == WM_DELETE_WINDOW) {
                        quit = true;
                    }
                } break;

                case ConfigureNotify: {
                    const auto c = (XConfigureEvent*)&e;
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

        glBegin (GL_TRIANGLES);
            glColor3f (1, 0, 0);
            glVertex3f (-1, -1, 0);
            glColor3f (0, 1, 0);
            glVertex3f (1, -1, 0);
            glColor3f (0, 0, 1);
            glVertex3f (0, 1, 0);
        glEnd ();

        glXSwapBuffers (display, xwindow);
    }

    return 0;
}
