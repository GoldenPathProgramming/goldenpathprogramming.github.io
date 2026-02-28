#define UNICODE
#define _UNICODE
#include <windows.h>
#include <assert.h>
#include <stdio.h>

int main() {
    puts ("Hello");

    const wchar_t window_class_name[] = L"Window Class";
    const WNDCLASS window_class = {
        .lpfnWndProc = DefWindowProc,
        .lpszClassName = window_class_name,
        .style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
        .hCursor = LoadCursor (NULL, IDC_ARROW),
    };
    { const auto result = RegisterClass (&window_class); assert (result); }

    const auto window_handle = CreateWindow (window_class_name, L"Golden Path", WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, NULL, NULL); assert (window_handle);

    MSG message;
    while (PeekMessage (&message, NULL, 0, 0, PM_REMOVE)) DispatchMessage (&message);

    Sleep (5000);
    return 0;
}