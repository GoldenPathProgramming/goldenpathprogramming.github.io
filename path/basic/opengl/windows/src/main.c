#define UNICODE
#define _UNICODE
#define WINVER _WIN32_WINNT_WIN10
#define _WIN32_WINNT _WIN32_WINNT_WIN10
#include <windows.h>
#include <GL/gl.h>
#include <assert.h>
#include <stdio.h>

static bool quit = false;
static LRESULT CALLBACK WindowProc(HWND window_handle, UINT message, WPARAM wParam, LPARAM lParam);

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

    const auto window_context = GetDC (window_handle);
    const PIXELFORMATDESCRIPTOR format_descriptor = {
        .nSize = sizeof (PIXELFORMATDESCRIPTOR),
        .nVersion = 1,
        .dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        .iPixelType = PFD_TYPE_RGBA,
        .cColorBits = 24,
        .cRedBits = 8, .cGreenBits = 8, .cBlueBits = 8, .cAlphaBits = 8,
		.cDepthBits = 24, .cStencilBits = 8,
    };
    const int pixel_format = ChoosePixelFormat (window_context, &format_descriptor); assert (pixel_format);
    { const auto result = SetPixelFormat (window_context, pixel_format, &format_descriptor); assert (result); }

    const auto glcontext = wglCreateContext (window_context); assert (glcontext);
    { const auto result = wglMakeCurrent (window_context, glcontext); assert (result); }
    printf ("OpenGL version: %s\n", glGetString (GL_VERSION));

    glClearColor (0, 0, 0, 0);

    while (!quit) {
        MSG message;
        while (PeekMessage (&message, NULL, 0, 0, PM_REMOVE)) {
            if (message.message == WM_QUIT) {
                quit = true;
            }
            else DispatchMessage (&message);
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

        SwapBuffers (window_context);

        Sleep (10);
    }

    return 0;
}

static LRESULT CALLBACK WindowProc(HWND window_handle, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_DESTROY: PostQuitMessage (0); break;

        case WM_PAINT: ValidateRect (window_handle, NULL); break;

        case WM_SIZE: {
            glViewport (0, 0, LOWORD(lParam), HIWORD(lParam));
            glMatrixMode (GL_PROJECTION);
            glLoadIdentity ();
         } break;

        default: return DefWindowProc (window_handle, message, wParam, lParam);
    }
    return 0;
}