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

    const auto window = XCreateWindow(display, root_window, 0, 0, 640, 480, 0, visual.depth, InputOutput, visual.visual, CWBackPixel | CWColormap | CWEventMask, &attributes);

    #define WINDOW_TITLE "Golden Path"
    XChangeProperty (display, window, XInternAtom (display, "_NET_WM_NAME", False), XInternAtom (display, "UTF8_STRING", False), 8, 0, (const unsigned char*)WINDOW_TITLE, sizeof (WINDOW_TITLE)-1);

    Atom WM_DELETE_WINDOW = XInternAtom (display, "WM_DELETE_WINDOW", False);
    { const auto result = XSetWMProtocols (display, window, &WM_DELETE_WINDOW, 1); assert (result); }

    XMapWindow (display, window);
    XFlush (display);

    XkbSetDetectableAutoRepeat (display, True, 0); // No key release events when holding key down

    bool quit = false;
    while (!quit) {
        XEvent e;
        XNextEvent (display, &e);
        printf ("Event: ");

        bool pressed = true;

        switch (e.type) {
            case DestroyNotify: {
                puts ("DestroyNotify");
                const auto d = &e.xdestroywindow;
                if (d->window == window) quit = true;
            } break;

            case ClientMessage: {
                printf ("ClientMessage: ");
                const auto c = &e.xclient;
                if ((Atom)c->data.l[0] == WM_DELETE_WINDOW) {
                    puts ("WM_DELETE_WINDOW");
                    quit = true;
                }
            } break;

            case FocusOut: {
                puts ("FocusOut");
            } break;

            case KeyRelease: pressed = false;
            case KeyPress: {
                const auto k = &e.xkey;
                char c;
                KeySym sym;
                const auto length = XLookupString (k, &c, 1, &sym, NULL);
                const auto unmodified_sym = XLookupKeysym (k, 0);
                if (length && c >= 33 && c <= 126)
                    printf ("Key%s [%c]\n", pressed ? "Press" : "Release", c);
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
                    printf ("Key%s symbol [%s]\n", pressed ? "Press" : "Release", symstr);
                }
            } break;

            case ConfigureNotify: {
                const auto c = &e.xconfigure;
                printf ("ConfigureNotify: x[%d] y[%d] w[%d] h[%d]\n", c->x, c->y, c->width, c->height);
            } break;

            case MotionNotify: {
                const auto m = &e.xmotion;
                printf ("MotionNotify: x[%d] y[%d]\n", m->x, m->y);
            } break;

            case ButtonRelease: pressed = false;
            case ButtonPress: {
                const auto b = &e.xbutton;
                const char *name = "Other";
                switch (b->button) {
                    case Button1: name = "Left"; break;
                    case Button2: name = "Middle"; break;
                    case Button3: name = "Right"; break;
                    case Button4: name = "Scrollup"; break;
                    case Button5: name = "Scrolldown"; break;
                }
                printf ("Button%s: %s\n", pressed ? "Press" : "Release", name);
            } break;

            default: puts ("Unknown"); break;
        }
    }

    return 0;
}
