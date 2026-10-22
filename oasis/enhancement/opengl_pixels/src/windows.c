// Compile with:
// gcc windows.c -lgdi32 -lopengl32

#define UNICODE
#define _UNICODE
#include <windows.h>
#include <GL/gl.h>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>

static bool quit = false;

#define WGL_CONTEXT_MAJOR_VERSION_ARB               0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB               0x2092
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB   0x00000002
#define WGL_CONTEXT_PROFILE_MASK_ARB                0x9126
typedef char GLchar;

typedef union {
    struct { uint8_t major, minor; };
    uint16_t full;
} glversion_t;

bool GLHasExtension (const char *const extension) {
    const char *gl_extensions = (const char*)glGetString (GL_EXTENSIONS);

    const auto len = strlen (extension);
    for (;;) {
        gl_extensions = strstr (gl_extensions, extension);
        if (gl_extensions == NULL) return false;
        char next = *(gl_extensions + len);
        if (next == ' ' || next == '\0') return true;
        gl_extensions += len;
    }
    return false;
}

static LRESULT CALLBACK WindowProc(HWND window_handle, UINT message, WPARAM wParam, LPARAM lParam);

static struct {
    int32_t w, h;
} window = {.w = 640, .h = 480};

typedef struct {
    uint8_t r, g, b;
} pixel_t;

struct {
    int32_t width, height;
    pixel_t *pixels;
} pixel_buffer;

static void OnResize () {
    glViewport (0, 0, window.w, window.h);
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();

    pixel_buffer.width = window.w > 0 ? window.w : 1;
    pixel_buffer.height = window.h > 0 ? window.h : 1;
    pixel_buffer.pixels = (pixel_t*)realloc (pixel_buffer.pixels, pixel_buffer.width * pixel_buffer.height * sizeof (pixel_buffer.pixels[0]));
    assert (pixel_buffer.pixels);
    memset (pixel_buffer.pixels, 0, pixel_buffer.width * pixel_buffer.height * sizeof (pixel_buffer.pixels[0]));

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, pixel_buffer.width, pixel_buffer.height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
}

int main () {
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

    const auto gl_context_temp = wglCreateContext (window_context); assert (gl_context_temp);
    { const auto result = wglMakeCurrent (window_context, gl_context_temp); assert (result); }

    typedef HGLRC (*wglCreateContextAttribsARB_t) (HDC hDC, HGLRC hshareContext, const int *attribList); 
    const auto wglCreateContextAttribsARB = (wglCreateContextAttribsARB_t)wglGetProcAddress ((LPCSTR)"wglCreateContextAttribsARB"); assert (wglCreateContextAttribsARB);

    const int context_attributes[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 2,
        WGL_CONTEXT_MINOR_VERSION_ARB, 1,
        0
    };

    const auto gl_context = wglCreateContextAttribsARB (window_context, 0, context_attributes);
    assert (gl_context);
    
    { const auto result = wglMakeCurrent (window_context, gl_context); assert (result); }

    wglDeleteContext (gl_context_temp);

    const char *gl_version = (const char*)glGetString (GL_VERSION);

    glversion_t glversion = {.major = gl_version[0] - '0', .minor = gl_version[2] - '0'};
    bool glversion_compatibility = true;
    if (glversion.full >= (glversion_t){.major=3,.minor=2}.full) { // Verion 3.2 onward have core and compatibility profiles. Core removes deprecated functionality.
        int context_profile = 0;
        glGetIntegerv (WGL_CONTEXT_PROFILE_MASK_ARB, &context_profile);
        if (!(context_profile & WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB)) glversion_compatibility = false;
    }
    else if (glversion.full == (glversion_t){.major=3,.minor=1}.full) { // Version 3.1 removed deprecated functionality, but did not have profiles yet.
        if (!GLHasExtension("GL_ARB_compatibility")) glversion_compatibility = false;
    }

    glClearColor (0, 0, 0, 0);

    if (glversion.full < (glversion_t){.major = 2, .minor = 1}.full || (glversion.full > (glversion_t){.major = 2, .minor = 1}.full && !glversion_compatibility)) {
        puts ("OpenGL context 2.1, or >2.1 with compatibility profile required.");
        return 1;
    }

    glEnable (GL_TEXTURE_2D);

    GLuint tex;
    glGenTextures (1, &tex);
    glBindTexture (GL_TEXTURE_2D, tex);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    OnResize ();

    while (!quit) {
        MSG message;
        while (PeekMessage (&message, NULL, 0, 0, PM_REMOVE)) {
            if (message.message == WM_QUIT) quit = true;
            else DispatchMessage (&message);
        }
        
        glClear (GL_COLOR_BUFFER_BIT);

        static uint8_t r = 0;
        static int8_t rsign = 1;
        r += rsign;
        if (r == 255) rsign = -1;
        else if (r == 0) rsign = 1;

        for (int y = 0; y < pixel_buffer.height; ++y) {
            uint8_t g = y * 255 / pixel_buffer.height;
            for (int x = 0; x < pixel_buffer.width; ++x) {
                uint8_t b = x * 255 / pixel_buffer.width;

                const auto p = &pixel_buffer.pixels[x + y * pixel_buffer.width];
                p->r = r;
                p->g = g;
                p->b = b;
            }
        }

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	    glTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, pixel_buffer.width, pixel_buffer.height, GL_RGB, GL_UNSIGNED_BYTE, pixel_buffer.pixels);

        glBegin (GL_TRIANGLE_STRIP);
            glTexCoord2i (0, 1);
            glVertex3f (-1,  1, 0);
            glTexCoord2i (0, 0);
            glVertex3f (-1, -1, 0);
            glTexCoord2i (1, 1);
            glVertex3f ( 1, 1, 0);
            glTexCoord2i (1, 0);
            glVertex3f ( 1, -1, 0);
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
            window.w = LOWORD(lParam);
            window.h = HIWORD(lParam);

            OnResize ();
         } break;

        default: return DefWindowProc (window_handle, message, wParam, lParam);
    }
    return 0;
}