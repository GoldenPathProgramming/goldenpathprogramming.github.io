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
#define GL_FRAGMENT_SHADER                          0x8B30
#define GL_VERTEX_SHADER                            0x8B31
#define GL_COMPILE_STATUS                           0x8B81
#define GL_LINK_STATUS                              0x8B82
#define GL_VALIDATE_STATUS                          0x8B83
typedef char GLchar;

typedef GLuint (*glCreateShader_t) (GLenum shaderType); glCreateShader_t glCreateShader;
typedef void (*glShaderSource_t) (GLuint shader, GLsizei count, const GLchar **string, const GLint *length); glShaderSource_t glShaderSource;
typedef void (*glCompileShader_t) (GLuint shader); glCompileShader_t glCompileShader;
typedef void (*glGetShaderiv_t) (GLuint shader, GLenum pname, GLint *params); glGetShaderiv_t glGetShaderiv;
typedef void (*glGetShaderInfoLog_t) (GLuint shader, GLsizei maxLength, GLsizei *length, GLchar *infoLog); glGetShaderInfoLog_t glGetShaderInfoLog;
typedef GLuint (*glCreateProgram_t) (); glCreateProgram_t glCreateProgram;
typedef void (*glAttachShader_t) (GLuint program, GLuint shader); glAttachShader_t glAttachShader;
typedef void (*glLinkProgram_t) (GLuint program); glLinkProgram_t glLinkProgram;
typedef void (*glGetProgramInfoLog_t) (GLuint program, GLsizei maxLength, GLsizei *length, GLchar *infoLog); glGetProgramInfoLog_t glGetProgramInfoLog;
typedef void (*glDeleteShader_t) (GLuint shader); glDeleteShader_t glDeleteShader;
typedef void (*glUseProgram_t) (GLuint program); glUseProgram_t glUseProgram;
typedef void (*glValidateProgram_t) (GLuint program); glValidateProgram_t glValidateProgram;
typedef void (*glGetProgramiv_t) (GLuint program, GLenum pname, GLint *params); glGetProgramiv_t glGetProgramiv;

#define GLFUNC(__funcname__) do { __funcname__ = (__funcname__##_t)wglGetProcAddress ((const GLubyte*)#__funcname__); assert (__funcname__); if (__funcname__ == NULL) { puts ("WGL failed to find function ["#__funcname__"]"); return false; } } while (0)

bool GetGLFuncs () {
    GLFUNC (glCreateShader);
    GLFUNC (glShaderSource);
    GLFUNC (glCompileShader);
    GLFUNC (glGetShaderiv);
    GLFUNC (glGetShaderInfoLog);
    GLFUNC (glCreateProgram);
    GLFUNC (glAttachShader);
    GLFUNC (glLinkProgram);
    GLFUNC (glGetProgramInfoLog);
    GLFUNC (glDeleteShader);
    GLFUNC (glUseProgram);
    GLFUNC (glValidateProgram);
    GLFUNC (glGetProgramiv);
    return true;
}

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

static LRESULT CALLBACK WindowProc(HWND window_handle, UINT message, WPARAM wParam, LPARAM lParam);

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
    const char *gl_extensions = (const char*)glGetString (GL_EXTENSIONS);
    const char *wgl_extensions = gl_extensions;

    typedef const char* (*wglGetExtensionsStringEXT_t) (void);
    const auto wglGetExtensionsStringEXT = (wglGetExtensionsStringEXT_t)wglGetProcAddress ("wglGetExtensionsStringEXT");
    if (wglGetExtensionsStringEXT) wgl_extensions = wglGetExtensionsStringEXT ();

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
    glClearColor (0, 0, 0, 0);

    if (glversion.full < (glversion_t){.major = 2, .minor = 1}.full || (glversion.full > (glversion_t){.major = 2, .minor = 1}.full && !glversion_compatibility)) {
        puts ("OpenGL context 2.1, or >2.1 with compatibility profile required.");
        return 1;
    }

    if (!GetGLFuncs ()) {
        puts ("Failed to retrieve OpenGL functions.");
        return 1;
    };

    const auto vertex = glCreateShader (GL_VERTEX_SHADER);
    glShaderSource (vertex, 1, &(const char*){
R"(#version 120

varying vec4 color;

void main()
{
    vec4 scale = vec4(1, 0.25, 1, 1);
    gl_Position = gl_ModelViewProjectionMatrix * scale * gl_Vertex;
    color = gl_Color;
})"}, NULL);

    glCompileShader (vertex);
    int success = 0;
    glGetShaderiv (vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        char buf[512];
        glGetShaderInfoLog (vertex, sizeof (buf), NULL, buf);
        printf ("OpenGL vertex shader compilation error: %s\n", buf);
        return -1;
    }

    const auto fragment = glCreateShader (GL_FRAGMENT_SHADER);
    glShaderSource (fragment, 1, &(const char *){
R"(#version 120

varying vec4 color;

void main()
{
    vec4 color_scale = vec4 (1, 0, 1, 1);
    gl_FragColor = color * color_scale;
}
)"}, NULL);

    glCompileShader (fragment);
    glGetShaderiv (fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        char buf[512];
        glGetShaderInfoLog (fragment, sizeof (buf), NULL, buf);
        printf ("OpenGL fragment shader compilation error: %s\n", buf);
        return -1;
    }

    const auto program = glCreateProgram ();
    glAttachShader (program, vertex);
    glAttachShader (program, fragment);
    glLinkProgram (program);
    glGetShaderiv (program, GL_LINK_STATUS, &success);
    if (!success) {
        char buf[512];
        glGetProgramInfoLog (program, sizeof (buf), NULL, buf);
        printf ("OpenGL shader linking error: %s\n", buf);
        return -1;
    }

    glDeleteShader (vertex);
    glDeleteShader (fragment);
    glUseProgram (program);
    glValidateProgram (program);
	glGetProgramiv (program, GL_VALIDATE_STATUS, &success);
	if (!success) {
		char buf[512];
		glGetProgramInfoLog (program, 512, NULL, buf);
		printf ("OpenGL shader validation error [%s]", buf);
		return -1;
	}

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