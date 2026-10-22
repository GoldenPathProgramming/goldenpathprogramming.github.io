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

    // Inside main(), before mainloop
    glEnable (GL_TEXTURE_2D);

    GLuint tex;
    glGenTextures (1, &tex);
    glBindTexture (GL_TEXTURE_2D, tex);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    OnResize ();

        // Rendering at the end of mainloop
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