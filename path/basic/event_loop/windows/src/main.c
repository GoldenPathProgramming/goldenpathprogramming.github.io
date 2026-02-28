#define UNICODE
#define _UNICODE
#define WINVER _WIN32_WINNT_WIN10
#define _WIN32_WINNT WINVER
#include <windows.h>
#include <assert.h>
#include <stdio.h>

static bool quit = false;
static LRESULT CALLBACK WindowProc(HWND window_handle, UINT message, WPARAM wParam, LPARAM lParam);
static const char *vk_print[0xff];

int main() {
    const wchar_t window_class_name[] = L"Window Class";
    const WNDCLASS window_class = {
        .lpfnWndProc = WindowProc,
        .lpszClassName = window_class_name,
        .style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
        .hCursor = LoadCursor (NULL, IDC_ARROW),
    };
    { const auto result = RegisterClass (&window_class); assert (result); }

    const auto window_handle = CreateWindow (window_class_name, L"Golden Path", WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, NULL, NULL); assert (window_handle);

    while (!quit) {
        MSG message;
        while (PeekMessage (&message, NULL, 0, 0, PM_REMOVE)) {
            if (message.message == WM_QUIT) {
                puts ("Quitting");
                quit = true;
            }
            else DispatchMessage (&message);
        }
    }

    return 0;
}

static LRESULT CALLBACK WindowProc(HWND window_handle, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_DESTROY: PostQuitMessage (0); break;

        case WM_PAINT: ValidateRect (window_handle, NULL); break; // You might do rendering here

        case WM_SIZE: printf ("Window resized: %d, %d\n", LOWORD(lParam), HIWORD(lParam)); break;

        case WM_KILLFOCUS: puts ("Focus lost"); break;

        case WM_SETFOCUS: puts ("Focus gained"); break;

        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYUP: {
            bool key_down = ((lParam & (1 << 31)) == 0);
            if (key_down && (lParam & (1 << 30))) {
                puts ("Key repeat");
                break;
            }
            printf ("Key %s %s\n", key_down ? "pressed" : "released", vk_print[wParam & 0xff]);
        }

        case WM_MOUSEMOVE: printf ("Mouse move: %d, %d\n", LOWORD(lParam), HIWORD(lParam)); break;

        case WM_LBUTTONDOWN: puts ("Mouse left pressed"); break;
        case WM_LBUTTONUP: puts ("Mouse left released"); break;
        case WM_RBUTTONDOWN: puts ("Mouse right pressed"); break;
        case WM_RBUTTONUP: puts ("Mouse right released"); break;
        case WM_MBUTTONDOWN: puts ("Mouse middle pressed"); break;
        case WM_MBUTTONUP: puts ("Mouse middle released"); break;
        case WM_XBUTTONDOWN: printf ("Mouse X%d pressed", GET_XBUTTON_WPARAM (wParam)); break;
        case WM_XBUTTONUP: printf ("Mouse X%d released", GET_XBUTTON_WPARAM (wParam)); break;

        case WM_MOUSEWHEEL: printf ("Scroll %s\n", GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? "up" : "down"); break;

        default: return DefWindowProc (window_handle, message, wParam, lParam);
    }
    return 0;
}

static const char *vk_print[0xff] = {
    [VK_LBUTTON] = "Left mouse", [VK_RBUTTON] = "Right mouse", [VK_CANCEL] = "Control-break processing", [VK_MBUTTON] = "Middle mouse", [VK_XBUTTON1] = "X1 mouse", [VK_XBUTTON2] = "X2 mouse", [0x07] = "Reserved", [VK_BACK] = "Backspace", [VK_TAB] = "Tab", [0x0A] = "Reserved", [VK_CLEAR] = "Clear", [VK_RETURN] = "Enter", [0x0E] = "Unassigned", [VK_SHIFT] = "Shift", [VK_CONTROL] = "Ctrl", [VK_MENU] = "Alt", [VK_PAUSE] = "Pause", [VK_CAPITAL] = "Caps lock", [VK_KANA] = "IME Kana mode", [VK_HANGUL] = "IME Hangul mode", [VK_IME_ON] = "IME On", [VK_JUNJA] = "IME Junja mode", [VK_FINAL] = "IME final mode", [VK_HANJA] = "IME Hanja mode", [VK_KANJI] = "IME Kanji mode", [VK_IME_OFF] = "IME Off", [VK_ESCAPE] = "Esc", [VK_CONVERT] = "IME convert", [VK_NONCONVERT] = "IME nonconvert", [VK_ACCEPT] = "IME accept", [VK_MODECHANGE] = "IME mode change request", [VK_SPACE] = "Spacebar", [VK_PRIOR] = "Page up", [VK_NEXT] = "Page down", [VK_END] = "End", [VK_HOME] = "Home", [VK_LEFT] = "Left arrow", [VK_UP] = "Up arrow", [VK_RIGHT] = "Right arrow", [VK_DOWN] = "Down arrow", [VK_SELECT] = "Select", [VK_PRINT] = "Print", [VK_EXECUTE] = "Execute", [VK_SNAPSHOT] = "Print screen", [VK_INSERT] = "Insert", [VK_DELETE] = "Delete", [VK_HELP] = "Help", ['0'] = "0", ['1'] = "1", ['2'] = "2", ['3'] = "3", ['4'] = "4", ['5'] = "5", ['6'] = "6", ['7'] = "7", ['8'] = "8", ['9'] = "9", [0x3A] = "Undefined", ['A'] = "A", ['B'] = "B", ['C'] = "C", ['D'] = "D", ['E'] = "E", ['F'] = "F", ['G'] = "G", ['H'] = "H", ['I'] = "I", ['J'] = "J", ['K'] = "K", ['L'] = "L", ['M'] = "M", ['N'] = "N", ['O'] = "O", ['P'] = "P", ['Q'] = "Q", ['R'] = "R", ['S'] = "S", ['T'] = "T", ['U'] = "U", ['V'] = "V", ['W'] = "W", ['X'] = "X", ['Y'] = "Y", ['Z'] = "Z", [VK_LWIN] = "Left Windows logo", [VK_RWIN] = "Right Windows logo", [VK_APPS] = "Application", [0x5E] = "Reserved", [VK_SLEEP] = "Computer Sleep", [VK_NUMPAD0] = "Numeric keypad 0", [VK_NUMPAD1] = "Numeric keypad 1", [VK_NUMPAD2] = "Numeric keypad 2", [VK_NUMPAD3] = "Numeric keypad 3", [VK_NUMPAD4] = "Numeric keypad 4", [VK_NUMPAD5] = "Numeric keypad 5", [VK_NUMPAD6] = "Numeric keypad 6", [VK_NUMPAD7] = "Numeric keypad 7", [VK_NUMPAD8] = "Numeric keypad 8", [VK_NUMPAD9] = "Numeric keypad 9", [VK_MULTIPLY] = "Multiply", [VK_ADD] = "Add", [VK_SEPARATOR] = "Separator", [VK_SUBTRACT] = "Subtract", [VK_DECIMAL] = "Decimal", [VK_DIVIDE] = "Divide", [VK_F1] = "F1", [VK_F2] = "F2", [VK_F3] = "F3", [VK_F4] = "F4", [VK_F5] = "F5", [VK_F6] = "F6", [VK_F7] = "F7", [VK_F8] = "F8", [VK_F9] = "F9", [VK_F10] = "F10", [VK_F11] = "F11", [VK_F12] = "F12", [VK_F13] = "F13", [VK_F14] = "F14", [VK_F15] = "F15", [VK_F16] = "F16", [VK_F17] = "F17", [VK_F18] = "F18", [VK_F19] = "F19", [VK_F20] = "F20", [VK_F21] = "F21", [VK_F22] = "F22", [VK_F23] = "F23", [VK_F24] = "F24", [0x88] = "Reserved", [VK_NUMLOCK] = "Num lock", [VK_SCROLL] = "Scroll lock", [0x92] = "OEM specific", [0x97] = "Unassigned", [VK_LSHIFT] = "Left Shift", [VK_RSHIFT] = "Right Shift", [VK_LCONTROL] = "Left Ctrl", [VK_RCONTROL] = "Right Ctrl", [VK_LMENU] = "Left Alt", [VK_RMENU] = "Right Alt", [VK_BROWSER_BACK] = "Browser Back", [VK_BROWSER_FORWARD] = "Browser Forward", [VK_BROWSER_REFRESH] = "Browser Refresh", [VK_BROWSER_STOP] = "Browser Stop", [VK_BROWSER_SEARCH] = "Browser Search", [VK_BROWSER_FAVORITES] = "Browser Favorites", [VK_BROWSER_HOME] = "Browser Start and Home", [VK_VOLUME_MUTE] = "Volume Mute", [VK_VOLUME_DOWN] = "Volume Down", [VK_VOLUME_UP] = "Volume Up", [VK_MEDIA_NEXT_TRACK] = "Next Track", [VK_MEDIA_PREV_TRACK] = "Previous Track", [VK_MEDIA_STOP] = "Stop Media", [VK_MEDIA_PLAY_PAUSE] = "Play/Pause Media", [VK_LAUNCH_MAIL] = "Start Mail", [VK_LAUNCH_MEDIA_SELECT] = "Select Media", [VK_LAUNCH_APP1] = "Start Application 1", [VK_LAUNCH_APP2] = "Start Application 2", [0xB8] = "Reserved", [VK_OEM_1] = "It can vary by keyboard. For the US ANSI keyboard , the Semicolon and Colon", [VK_OEM_PLUS] = "Equals and Plus", [VK_OEM_COMMA] = "Comma and Less Than", [VK_OEM_MINUS] = "Dash and Underscore", [VK_OEM_PERIOD] = "Period and Greater Than", [VK_OEM_2] = "It can vary by keyboard. For the US ANSI keyboard, the Forward Slash and Question Mark", [VK_OEM_3] = "It can vary by keyboard. For the US ANSI keyboard, the Grave Accent and Tilde", [0xC1] = "Reserved", [VK_GAMEPAD_A] = "Gamepad A", [VK_GAMEPAD_B] = "Gamepad B", [VK_GAMEPAD_X] = "Gamepad X", [VK_GAMEPAD_Y] = "Gamepad Y", [VK_GAMEPAD_RIGHT_SHOULDER] = "Gamepad Right Shoulder", [VK_GAMEPAD_LEFT_SHOULDER] = "Gamepad Left Shoulder", [VK_GAMEPAD_LEFT_TRIGGER] = "Gamepad Left Trigger", [VK_GAMEPAD_RIGHT_TRIGGER] = "Gamepad Right Trigger", [VK_GAMEPAD_DPAD_UP] = "Gamepad D-pad Up", [VK_GAMEPAD_DPAD_DOWN] = "Gamepad D-pad Down", [VK_GAMEPAD_DPAD_LEFT] = "Gamepad D-pad Left", [VK_GAMEPAD_DPAD_RIGHT] = "Gamepad D-pad Right", [VK_GAMEPAD_MENU] = "Gamepad Menu/Start", [VK_GAMEPAD_VIEW] = "Gamepad View/Back", [VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON] = "Gamepad Left Thumbstick", [VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON] = "Gamepad Right Thumbstick", [VK_GAMEPAD_LEFT_THUMBSTICK_UP] = "Gamepad Left Thumbstick up", [VK_GAMEPAD_LEFT_THUMBSTICK_DOWN] = "Gamepad Left Thumbstick down", [VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT] = "Gamepad Left Thumbstick right", [VK_GAMEPAD_LEFT_THUMBSTICK_LEFT] = "Gamepad Left Thumbstick left", [VK_GAMEPAD_RIGHT_THUMBSTICK_UP] = "Gamepad Right Thumbstick up", [VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN] = "Gamepad Right Thumbstick down", [VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT] = "Gamepad Right Thumbstick right", [VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT] = "Gamepad Right Thumbstick left", [VK_OEM_4] = "It can vary by keyboard. For the US ANSI keyboard, the Left Brace", [VK_OEM_5] = "It can vary by keyboard. For the US ANSI keyboard, the Backslash and Pipe", [VK_OEM_6] = "It can vary by keyboard. For the US ANSI keyboard, the Right Brace", [VK_OEM_7] = "It can vary by keyboard. For the US ANSI keyboard, the Apostrophe and Double Quotation Mark", [VK_OEM_8] = "It can vary by keyboard. For the Canadian CSA keyboard, the Right Ctrl", [0xE0] = "Reserved", [0xE1] = "OEM specific", [VK_OEM_102] = "It can vary by keyboard. For the European ISO keyboard, the Backslash and Pipe", [0xE3] = "OEM specific", [VK_PROCESSKEY] = "IME PROCESS", [0xE6] = "OEM specific", [VK_PACKET] = "Used to pass Unicode characters as if they were keystrokes. The VK_PACKET key is the low word of a 32-bit Virtual Key value used for non-keyboard input methods. For more information, see Remark in KEYBDINPUT, SendInput, WM_KEYDOWN, and WM_KEYUP", [0xE8] = "Unassigned", [0xE9] = "OEM specific", [VK_ATTN] = "Attn", [VK_CRSEL] = "CrSel", [VK_EXSEL] = "ExSel", [VK_EREOF] = "Erase EOF", [VK_PLAY] = "Play", [VK_ZOOM] = "Zoom", [VK_NONAME] = "Reserved", [VK_PA1] = "PA1", [VK_OEM_CLEAR] = "Clear",
};
