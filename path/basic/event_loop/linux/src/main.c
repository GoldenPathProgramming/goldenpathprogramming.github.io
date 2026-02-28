#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/XKBlib.h>
#include <assert.h>
#include <stdio.h>

int main () {
    const auto display = XOpenDisplay (NULL);
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

    const auto window = XCreateWindow(display, root_window, 0, 0, 640, 480, 0, visual.depth, 0, visual.visual, CWBackPixel | CWColormap | CWEventMask, &attributes);

    XMapWindow (display, window);
    XFlush (display);

    #define WINDOW_TITLE "Golden Path"
    XChangeProperty (display, window, XA_WM_NAME, XA_STRING, 8, 0, (const unsigned char*)WINDOW_TITLE, sizeof (WINDOW_TITLE)-1);

    Atom WM_DELETE_WINDOW = XInternAtom (display, "WM_DELETE_WINDOW", False);
    if (WM_DELETE_WINDOW != None)
        { const auto result = XSetWMProtocols (display, window, &WM_DELETE_WINDOW, 1); assert (result); }

    XkbSetDetectableAutoRepeat (os_private.x11.display, True, 0); // No key repeat when holding key down

    bool quit = false;
    while (!quit) {
        XEvent e;
        XNextEvent (display, &e);
        printf ("Event: ");
        switch (e.type) {
            case DestroyNotify: {
                puts ("DestroyNotify");
                quit = true;
            } break;

            case ClientMessage: {
                printf ("ClientMessage: ");
                const auto c = (XClientMessageEvent*)&e;
                if (WM_DELETE_WINDOW && (Atom)c->data.l[0] == WM_DELETE_WINDOW) {
                    puts ("WM_DELETE_WINDOW");
                    quit = true;
                }
            } break;

            case FocusOut: {
                puts ("FocusOut");
            } break;

            case KeyPress: {
                const auto k = (XKeyPressedEvent*)&e;
                char c;
                KeySym sym;
                XLookupString (k, &c, 1, &sym, NULL);
                if (c >= 33 && c <= 126)
                    printf ("KeyPress [%c]\n", c);
                else {
                    const char *symstr = "Unknown";
                    switch (sym) {
                        case XK_Return: symstr = "Enter"; break;
                        case XK_space: symstr = "Space"; break;
                        case XK_Escape: symstr = "Escape"; break;
                        case XK_Left: symstr = "Left"; break;
                        case XK_Right: symstr = "Right"; break;
                        case XK_Up: symstr = "Up"; break;
                        case XK_Down: symstr = "Down"; break;
                    }
                    printf ("KeyPress symbol [%s]\n", symstr);
                }
            } break;

            case KeyRelease: {
                const auto k = (XKeyPressedEvent*)&e;
                const auto sym = XLookupKeysym (k, 0);
                if (sym >= 33 && sym <= 126)
                    printf ("KeyRelease [%c]\n", (char)sym);
                else
                    puts ("KeyRelease symbol");
            } break;

            case ConfigureNotify: {
                const auto c = (XConfigureEvent*)&e;
                printf ("ConfigureNotify: x[%d] y[%d] w[%d] h[%d]\n", c->x, c->y, c->width, c->height);
            } break;

            case MotionNotify: {
                const auto m = (XMotionEvent*)&e;
                printf ("MotionNotify: x[%d] y[%d]\n", m->x, m->y);
            } break;

            case ButtonPress: {
                const auto b = (XButtonPressedEvent*)&e;
                const char *name = "Other";
                switch (b->button) {
                    case Button1: name = "Left"; break;
                    case Button2: name = "Middle"; break;
                    case Button3: name = "Right"; break;
                    case Button4: name = "Scrollup"; break;
                    case Button5: name = "Scrolldown"; break;
                }
                printf ("ButtonPress: %s\n", name);
            } break;

            case ButtonRelease: {
                puts ("ButtonRelease");
            } break;

            default: puts ("Unknown"); break;
        }
    }

    return 0;
}
