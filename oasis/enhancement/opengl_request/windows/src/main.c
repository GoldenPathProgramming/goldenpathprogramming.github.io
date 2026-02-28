#define UNICODE
#define _UNICODE
#include <windows.h>
#include <GL/gl.h>
#include <assert.h>
#include <stdio.h>

#define WGL_CONTEXT_MAJOR_VERSION_ARB           0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB           0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB             0x2093
#define WGL_CONTEXT_FLAGS_ARB                   0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB            0x9126
#define WGL_CONTEXT_DEBUG_BIT_ARB               0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB  0x0002
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB        0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002

typedef HGLRC (*wglCreateContextAttribsARB_t) (HDC hDC, HGLRC hshareContext, const int *attribList); wglCreateContextAttribsARB_t wglCreateContextAttribsARB;

int main() {
    const wchar_t window_class_name[] = L"Window Class";
    const WNDCLASS window_class = {
        .lpfnWndProc = DefWindowProc,
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

    const auto gl_context_temp = wglCreateContext (window_context); assert (gl_context_temp);
    { const auto result = wglMakeCurrent (window_context, gl_context_temp); assert (result); }

    wglCreateContextAttribsARB = (wglCreateContextAttribsARB_t)wglGetProcAddress ((LPCSTR)"wglCreateContextAttribsARB"); assert (wglCreateContextAttribsARB);

    const int context_attributes[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 2,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0
    };

    const auto gl_context = wglCreateContextAttribsARB (window_context, 0, context_attributes);
    assert (gl_context);
    
    { const auto result = wglMakeCurrent (window_context, gl_context); assert (result); }

    wglDeleteContext (gl_context_temp);

    printf ("OpenGL version: %s\n", glGetString (GL_VERSION));

    return 0;
}
