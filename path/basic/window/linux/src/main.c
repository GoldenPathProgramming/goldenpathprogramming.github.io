#include <X11/Xlib.h>
#include <unistd.h>
#include <assert.h>

int main () {
    const auto display = XOpenDisplay (NULL);
    assert (display);

    const auto root_window = DefaultRootWindow(display);
    const auto screen = DefaultScreen (display);

    const auto window = XCreateSimpleWindow(display, root_window, 0, 0, 640, 480, 0, 0, 0x403a4d);

    XMapWindow (display, window);
    XFlush (display);

    sleep (5);
    return 0;
}
