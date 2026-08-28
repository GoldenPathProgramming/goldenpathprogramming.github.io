// Non-MacOS:
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

// All platforms:

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

// Inside an event loop:
        glBegin (GL_TRIANGLES);
            glColor3f (1, 0, 0);
            glVertex3f (-1, -1, 0);
            glColor3f (0, 1, 0);
            glVertex3f (1, -1, 0);
            glColor3f (0, 0, 1);
            glVertex3f (0, 1, 0);
        glEnd ();