// Compile with:
// gcc linux.c -lGL

#include <X11/Xatom.h>
#include <GL/glx.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

static struct {
    int16_t w, h;
} window = {.w = 640, .h = 480};

static void OnResize () {
    glViewport (0, 0, window.w, window.h);
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
}

typedef union {
    struct { uint8_t major, minor; };
    uint16_t full;
} glversion_t;

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

#define GLFUNC(__funcname__) do { __funcname__ = (__funcname__##_t)glXGetProcAddress ((const GLubyte*)#__funcname__); assert (__funcname__); if (__funcname__ == NULL) { puts ("GLX failed to find function ["#__funcname__"]"); return false; } } while (0)

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
    const auto display = XOpenDisplay (NULL);
    assert (display);

    const auto root_window = DefaultRootWindow(display);
    const auto screen = DefaultScreen (display);

    const int fb_attributes[] = {
        GLX_X_RENDERABLE, True,
        GLX_DOUBLEBUFFER, True,
        GLX_RED_SIZE, 8, GLX_GREEN_SIZE, 8, GLX_BLUE_SIZE, 8, GLX_DEPTH_SIZE, 24,
        None
    };

    int fb_count;
    GLXFBConfig* fb_configs = glXChooseFBConfig(display, screen, fb_attributes, &fb_count); assert(fb_configs && fb_count > 0);
    GLXFBConfig fb_config = fb_configs[0];
    XFree (fb_configs);

    XVisualInfo* visual = glXGetVisualFromFBConfig(display, fb_config); assert(visual);

    auto gl_context_temp = glXCreateContext (display, visual, 0, true); assert (gl_context_temp);

    XSetWindowAttributes attributes = {
        .background_pixel = 0x403a4d,
        .colormap = XCreateColormap (display, root_window, visual->visual, AllocNone),
        .event_mask = StructureNotifyMask | KeyPressMask | KeyReleaseMask | FocusChangeMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask,
    };
    const auto xwindow = XCreateWindow(display, root_window, 0, 0, 640, 480, 0, visual->depth, 0, visual->visual, CWBackPixel | CWColormap | CWEventMask, &attributes);

    typedef GLXContext (*glXCreateContextAttribsARB_t) (Display *dpy, GLXFBConfig config, GLXContext share_context, Bool direct, const int *attrib_list);
    const auto glXCreateContextAttribsARB = (glXCreateContextAttribsARB_t)glXGetProcAddress ((const GLubyte*)"glXCreateContextAttribsARB"); assert (glXCreateContextAttribsARB);

    const int context_attributes[] = {
        GLX_RENDER_TYPE, GLX_RGBA_TYPE,
        GLX_CONTEXT_MAJOR_VERSION_ARB, 2,
        GLX_CONTEXT_MINOR_VERSION_ARB, 1,
        None
    };

    const auto gl_context = glXCreateContextAttribsARB (display, fb_config, 0, True, context_attributes); assert(gl_context);

    { const auto result = glXMakeCurrent (display, xwindow, gl_context); assert (result); }

    glXDestroyContext (display, gl_context_temp);

    const char *gl_version = (const char*)glGetString (GL_VERSION);
    printf ("OpenGL version: %s\n", gl_version);

    glversion_t glversion = {.major = gl_version[0] - '0', .minor = gl_version[2] - '0'};
    bool glversion_compatibility = true;
    if (glversion.full >= (glversion_t){.major=3,.minor=2}.full) { // Verion 3.2 onward have core and compatibility profiles. Core removes deprecated functionality.
        int context_profile = 0;
        glGetIntegerv (GL_CONTEXT_PROFILE_MASK, &context_profile);
        if (!(context_profile & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT)) glversion_compatibility = false;
    }
    else if (glversion.full == (glversion_t){.major=3,.minor=1}.full) { // Version 3.1 removed deprecated functionality, but did not have profiles yet.
        if (!GLHasExtension(glGetString (GL_EXTENSIONS), "GL_ARB_compatibility")) glversion_compatibility = false;
    }

    XMapWindow (display, xwindow);
    XFlush (display);

    #define WINDOW_TITLE "Golden Path"
    XChangeProperty (display, xwindow, XA_WM_NAME, XA_STRING, 8, 0, (const unsigned char*)WINDOW_TITLE, sizeof (WINDOW_TITLE)-1);

    Atom WM_DELETE_WINDOW = XInternAtom (display, "WM_DELETE_WINDOW", False);
    if (WM_DELETE_WINDOW != None)
        { const auto result = XSetWMProtocols (display, xwindow, &WM_DELETE_WINDOW, 1); assert (result); }
    
    OnResize ();
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

    bool quit = false;
    while (!quit) {
        while (XPending (display)) {
            XEvent e;
            XNextEvent (display, &e);
            switch (e.type) {
                case DestroyNotify: {
                    quit = true;
                } break;

                case ClientMessage: {
                    const auto c = (XClientMessageEvent*)&e;
                    if (WM_DELETE_WINDOW && (Atom)c->data.l[0] == WM_DELETE_WINDOW) {
                        quit = true;
                    }
                } break;

                case ConfigureNotify: {
                    const auto c = (XConfigureEvent*)&e;
                    if (c->width != window.w || c->height != window.h) {
                        window.w = c->width;
                        window.h = c->height;
                        OnResize ();
                    }
                } break;

                default: break;
            }
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

        glXSwapBuffers (display, xwindow);
    }

    return 0;
}
