#define UNICODE
#define _UNICODE
#include <windows.h>
#include <GL/gl.h>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>

#define WGL_CONTEXT_MAJOR_VERSION_ARB               0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB               0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB                0x9126
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB   0x00000002
#define GL_VERTEX_SHADER                            0x8B31

typedef union {
    struct { uint8_t major, minor; };
    uint16_t full;
} glversion_t;

bool GLHasExtension (const char *const haystack, const char *const needle) {
    const char *e = haystack;
    const auto len = strlen (needle);
    for (;;) {
        e = strstr (e, needle);
        if (e == NULL) return false;
        char next = *(e + len);
        if (next == ' ' || next == '\0') return true;
        e += len;
    }
}

int main () {
    const wchar_t window_class_name[] = L"Window Class";
    const WNDCLASS window_class = {
        .lpfnWndProc = DefWindowProc,
        .lpszClassName = window_class_name,
        .style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
        .hCursor = LoadCursor (NULL, IDC_ARROW),
    };
    { const auto result = RegisterClass (&window_class); assert (result); }

    const auto window_handle = CreateWindow (window_class_name, L"Golden Path", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, NULL, NULL); assert (window_handle);

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
    printf ("OpenGL version: %s\n", gl_version);

    const char *gl_extensions = (const char*)glGetString (GL_EXTENSIONS);
    printf ("\nGL extensions: %s\n\n", gl_extensions);

    const char *wgl_extensions = gl_extensions;

    typedef const char* (*wglGetExtensionsStringEXT_t) (void);
    const auto wglGetExtensionsStringEXT = (wglGetExtensionsStringEXT_t)wglGetProcAddress ("wglGetExtensionsStringEXT");
    if (wglGetExtensionsStringEXT) {
        wgl_extensions = wglGetExtensionsStringEXT ();
        printf ("WGL extensions: %s\n\n", wgl_extensions);
    }
    else puts ("wglGetExtensionsStringEXT not found");

    glversion_t glversion = {.major = gl_version[0] - '0', .minor = gl_version[2] - '0'};
    bool glversion_compatibility = true;
    if (glversion.full >= (glversion_t){.major=3,.minor=2}.full) { // Verion 3.2 onward have core and compatibility profiles. Core removes deprecated functionality.
        int context_profile = 0;
        glGetIntegerv (WGL_CONTEXT_PROFILE_MASK_ARB, &context_profile);
        if (!(context_profile & WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB)) glversion_compatibility = false;
    }
    else if (glversion.full == (glversion_t){.major=3,.minor=1}.full) { // Version 3.1 removed deprecated functionality, but did not have profiles yet.
        if (!GLHasExtension(gl_extensions, "GL_ARB_compatibility")) glversion_compatibility = false;
    }

    while (glversion.full >= (glversion_t){.major = 2, .minor = 0}.full && glversion_compatibility) {
        typedef GLuint (*glCreateShader_t) (GLenum shaderType);
        
        const auto glCreateShader = (glCreateShader_t)wglGetProcAddress ((const GLubyte*)"glCreateShader");
        if (glCreateShader == NULL) {
            puts ("Failed to retrieve glCreateShader function pointer");
            break;
        }

        const auto vertex_shader = glCreateShader (GL_VERTEX_SHADER);
        printf ("glCreateShader found and called: %u\n", vertex_shader);
    break;}

    if (GLHasExtension (gl_extensions, "GL_ARB_vertex_buffer_object")) {
        do {
            typedef void (*glGenBuffersARB_t) (GLsizei n, GLuint *buffers);

            const auto glGenBuffersARB = (glGenBuffersARB_t)wglGetProcAddress ((const GLubyte*)"glGenBuffersARB");
            if (glGenBuffersARB == NULL) {
                puts ("Failed to retrieve glGenBuffersARB function pointer");
                break;
            }

            GLuint buffer = 0;
            glGenBuffersARB (1, &buffer);
            printf ("glGenBuffersARB found and called: %d\n", buffer);
        } while (false);
    }
    else puts ("Extension not found: GL_ARB_vertex_buffer_object");

    if (GLHasExtension (wgl_extensions, "WGL_EXT_swap_control_tear")) {
        do {
            typedef BOOL (*wglSwapIntervalEXT_t) (int interval);

            const auto wglSwapIntervalEXT = (wglSwapIntervalEXT_t)wglGetProcAddress((const GLubyte*)"wglSwapIntervalEXT");
            if (wglSwapIntervalEXT == NULL) {
                puts ("Failed to retrieve wglSwapIntervalEXT function pointer");
                break;
            }
            wglSwapIntervalEXT (-1);
            puts ("wglSwapIntervalEXT found and called");
        } while (false);
    }
    else puts ("Extension not found: WGL_EXT_swap_control_tear");

    return 0;
}