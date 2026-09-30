#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <assert.h>

static Display *display;
static Window window;

static struct {
    Atom wm_state, fullscreen;
} atoms;

bool IsFullscreen () {
    Atom actual_type;
    int actual_format;
    unsigned long nitems;
    unsigned long bytes_after;
    Atom *prop;
    
    const auto result = XGetWindowProperty(display, window, atoms.wm_state, 0, 1024, False, XA_ATOM, &actual_type, &actual_format, &nitems, &bytes_after, (unsigned char **)&prop);

    if (result != Success || actual_type != XA_ATOM || actual_format != 32) return false;

    bool is_fullscreen = false;
    for (unsigned long i = 0; i < nitems; ++i) {
        if (prop[i] == atoms.fullscreen) {
            is_fullscreen = true;
            break;
        }
    }

    XFree (prop);
    return is_fullscreen;
}

void ToggleFullscreen () {
    bool go_fullscreen = !IsFullscreen ();

    XEvent e = {};
    e.type = ClientMessage;
    e.xclient.window = window;
    e.xclient.message_type = atoms.wm_state;
    e.xclient.format = 32;
    e.xclient.data.l[0] = go_fullscreen;
    e.xclient.data.l[1] = atoms.fullscreen;
    e.xclient.data.l[2] = 0;
    e.xclient.data.l[3] = 1;

    XSendEvent (display, DefaultRootWindow(display), False, SubstructureRedirectMask | SubstructureNotifyMask, &e);
}

int main () {
    display = XOpenDisplay (NULL);
    assert (display);

    const auto root_window = DefaultRootWindow(display);
    const auto screen = DefaultScreen (display);

    XVisualInfo visual = {};
    { const auto result = XMatchVisualInfo (display, screen, 24, TrueColor, &visual); assert (result); }

    XSetWindowAttributes attributes = {
        .background_pixel = 0x403a4d,
        .colormap = XCreateColormap (display, root_window, visual.visual, AllocNone),
        .event_mask = StructureNotifyMask | KeyPressMask | KeyReleaseMask | FocusChangeMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask,
    };

    window = XCreateWindow(display, root_window, 0, 0, 640, 480, 0, visual.depth, InputOutput, visual.visual, CWBackPixel | CWColormap | CWEventMask, &attributes);

    XMapWindow (display, window);
    XFlush (display);

    #define WINDOW_TITLE "Golden Path"
    XChangeProperty (display, window, XA_WM_NAME, XA_STRING, 8, 0, (const unsigned char*)WINDOW_TITLE, sizeof (WINDOW_TITLE)-1);

    Atom WM_DELETE_WINDOW = XInternAtom (display, "WM_DELETE_WINDOW", False);
    if (WM_DELETE_WINDOW != None)
        { const auto result = XSetWMProtocols (display, window, &WM_DELETE_WINDOW, 1); assert (result); }

    atoms.wm_state = XInternAtom (display, "_NET_WM_STATE", False);
    atoms.fullscreen = XInternAtom (display, "_NET_WM_STATE_FULLSCREEN", False);

    bool quit = false;
    while (!quit) {
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

            case KeyPress: {
                const auto k = (XKeyPressedEvent*)&e;
                char c;
                KeySym sym;
                XLookupString (k, &c, 1, &sym, NULL);
                switch (sym) {
                    case XK_Return: ToggleFullscreen (); break;
                    case XK_Escape: quit = true; break;
                }
            } break;
        }
    }

    return 0;
}
