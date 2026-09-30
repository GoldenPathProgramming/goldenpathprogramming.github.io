#define UNICODE
#define _UNICODE
#define WINVER _WIN32_WINNT_WIN10
#define _WIN32_WINNT WINVER
#include <windows.h>
#include <assert.h>
#include <stdio.h>

static bool quit = false;
static HWND window_handle;
static RECT fullscreen_monitor_rect = {};

static LRESULT CALLBACK WindowProc(HWND window_handle, UINT message, WPARAM wParam, LPARAM lParam);

bool IsFullscreen () {
    const auto monitor = MonitorFromWindow (window_handle, MONITOR_DEFAULTTOPRIMARY);
    MONITORINFOEX info = {.cbSize = sizeof (MONITORINFOEX)};

    const auto result = GetMonitorInfo (monitor, (LPMONITORINFO)&info);
    assert (result);
    if (result) {
        fullscreen_monitor_rect = info.rcMonitor;
        printf ("Monitor used for fullscreen: %ls\n", info.szDevice);
    }
    else {
        printf ("Failed to get monitor info.");
        HWND desktop_handle = GetDesktopWindow();
        if (desktop_handle) GetWindowRect(desktop_handle, &fullscreen_monitor_rect);
        else { fullscreen_monitor_rect.left = 0; fullscreen_monitor_rect.top = 0; fullscreen_monitor_rect.right = 800; fullscreen_monitor_rect.bottom = 600; }
    }

    const auto style = GetWindowLongPtr (window_handle, GWL_STYLE);
    if (style & WS_CAPTION || style & WS_BORDER || style & WS_THICKFRAME || style & WS_DLGFRAME) return false;

    RECT window_rect;
    GetWindowRect (window_handle, &window_rect);

    if (EqualRect (&fullscreen_monitor_rect, &window_rect)) return true;
    else return false;
}

void ToggleFullscreen () {
    static WINDOWPLACEMENT window_placement = {.length = sizeof (WINDOWPLACEMENT)};

    if (!IsFullscreen ()) {
        GetWindowPlacement (window_handle, &window_placement);

        const auto style = GetWindowLongPtr (window_handle, GWL_STYLE);
        SetWindowLongPtr (window_handle, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);

        SetWindowPos (window_handle, NULL, fullscreen_monitor_rect.left, fullscreen_monitor_rect.top, fullscreen_monitor_rect.right - fullscreen_monitor_rect.left, fullscreen_monitor_rect.bottom - fullscreen_monitor_rect.top, SWP_NOOWNERZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW);
    }
    else {
        const auto style = GetWindowLongPtr (window_handle, GWL_STYLE);
        SetWindowLongPtr (window_handle, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);

        SetWindowPlacement (window_handle, &window_placement);
    }
}

int main() {
    const wchar_t window_class_name[] = L"Window Class";
    const WNDCLASS window_class = {
        .lpfnWndProc = WindowProc,
        .lpszClassName = window_class_name,
        .style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
        .hCursor = LoadCursor (NULL, IDC_ARROW),
    };
    { const auto result = RegisterClass (&window_class); assert (result); }

    window_handle = CreateWindow (window_class_name, L"Golden Path", WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, NULL, NULL); assert (window_handle);

    while (!quit) {
        MSG message;
        while (PeekMessage (&message, NULL, 0, 0, PM_REMOVE)) {
            if (message.message == WM_QUIT) quit = true;
            else DispatchMessage (&message);
        }

        Sleep (5);
    }

    return 0;
}

static LRESULT CALLBACK WindowProc(HWND window_handle, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_DESTROY: PostQuitMessage (0); break;

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint (window_handle, &ps);
            FillRect (hdc, &ps.rcPaint, (HBRUSH)GetStockObject(BLACK_BRUSH));
            EndPaint (window_handle, &ps);
        } break;

        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYUP: {
            bool key_down = ((lParam & (1 << 31)) == 0);
            if (key_down && (lParam & (1 << 30))) {
                break;
            }
            if (key_down) {
                switch (wParam) {
                    case VK_RETURN: ToggleFullscreen (); break;
                    case VK_ESCAPE: PostQuitMessage (0); break;
                }
            }
        }

        default: return DefWindowProc (window_handle, message, wParam, lParam);
    }
    return 0;
}
