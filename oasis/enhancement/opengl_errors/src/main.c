#include <GL/glu.h>

static bool GLHadError () {
    bool had_error = false;
    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR) {
        had_error = true;
        printf ("OpenGL error: %s\n", gluErrorString(error));
    }
    return had_error;
}