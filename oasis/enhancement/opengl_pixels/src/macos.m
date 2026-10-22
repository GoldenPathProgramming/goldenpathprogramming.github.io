// Build with:
// clang main.m -std=gnu2y -framework Cocoa -framework OpenGL
#define GL_SILENCE_DEPRECATION
#import <Cocoa/Cocoa.h>
#include <OpenGL/glu.h>

typedef struct {
    uint8_t r, g, b;
} pixel_t;

struct {
    int32_t width, height;
    pixel_t *pixels;
} pixel_buffer;

static bool quit = false;
static NSOpenGLContext *gl_context;
static NSApplication *app;
static NSWindow *window;

@interface Delegate : NSObject<NSApplicationDelegate, NSWindowDelegate> @end
@implementation Delegate {
    bool live_resizing;
}
-(NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication*)sender {
    quit = true;
    return NSTerminateCancel;
}
-(void)windowWillClose:(NSNotification*)notification { quit = true; }
-(void)windowDidResize:(NSNotification *)notification {
    if (live_resizing) return;
    [self Resize];
}
-(void)windowWillStartLiveResize:(NSNotification *)notification {
	live_resizing = true;
}
-(void)windowDidEndLiveResize:(NSNotification *)notification {
    live_resizing = false;
    [self Resize];
}
-(void)Resize {
    NSSize size = [[window contentView] convertRectToBacking:[[window contentView] bounds]].size;
    glViewport (0, 0, size.width, size.height);
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    [gl_context update];

    pixel_buffer.width = size.width > 0 ? size.width : 1;
    pixel_buffer.height = size.height > 0 ? size.height : 1;
    pixel_buffer.pixels = (pixel_t*)realloc (pixel_buffer.pixels, pixel_buffer.width * pixel_buffer.height * sizeof (pixel_buffer.pixels[0]));
    assert (pixel_buffer.pixels);
    memset (pixel_buffer.pixels, 0, pixel_buffer.width * pixel_buffer.height * sizeof (pixel_buffer.pixels[0]));

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, pixel_buffer.width, pixel_buffer.height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
}
@end

int main () {
    app = [NSApplication sharedApplication];
    [app setActivationPolicy:NSApplicationActivationPolicyRegular];

    NSMenu *menu_bar = [NSMenu new];
    NSMenuItem *menu_item_app = [NSMenuItem new];
    [menu_bar addItem:menu_item_app];
    [app setMainMenu:menu_bar];

    NSMenu *app_menu = [NSMenu new];
    [app_menu addItem:[[NSMenuItem alloc] initWithTitle:[@"Quit " stringByAppendingString:[[NSProcessInfo processInfo] processName]] action:@selector(terminate:) keyEquivalent:@"q"]];
    [menu_item_app setSubmenu:app_menu];

    window = [[NSWindow alloc] initWithContentRect:NSMakeRect(0,0,640,480) styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable backing:NSBackingStoreBuffered defer:YES];
    [window setReleasedWhenClosed:NO];
    [window setTitle:@"Golden Path"];
    [window setFrameAutosaveName:[window title]];
    
    Delegate *delegate = [Delegate new];
    [app setDelegate:delegate];
    [window setDelegate:delegate];
    
    [window makeKeyAndOrderFront:window];

    NSOpenGLPixelFormatAttribute gl_attributes[] = {
        NSOpenGLPFAColorSize, 24,
        NSOpenGLPFAAlphaSize, 8,
        NSOpenGLPFAClosestPolicy,
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFAAccelerated,
        NSOpenGLPFANoRecovery,
        NSOpenGLPFADepthSize, 24,
        NSOpenGLPFAStencilSize, 8,
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersionLegacy,
        0,
    };
    NSOpenGLPixelFormat *pixel_format = [[NSOpenGLPixelFormat alloc] initWithAttributes:gl_attributes];
    gl_context = [[NSOpenGLContext alloc] initWithFormat:pixel_format shareContext:nil];
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wdeprecated-declarations"
    [gl_context setView:[window contentView]];
    #pragma clang diagnostic pop
    [gl_context makeCurrentContext];

    if ([app respondsToSelector:@selector(activate)]) [(id)app activate];
    else [app activateIgnoringOtherApps:true];

    glClearColor (0, 0, 0, 0);

    glEnable (GL_TEXTURE_2D);

    GLuint tex;
    glGenTextures (1, &tex);
    glBindTexture (GL_TEXTURE_2D, tex);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    [delegate Resize];

    while (!quit) {
        @autoreleasepool {
            for (;;) {
                NSEvent *e = [app nextEventMatchingMask:NSEventMaskAny untilDate:[NSDate distantPast] inMode:NSDefaultRunLoopMode dequeue:YES];
                if (!e) break;
                [app sendEvent:e];
            }
            [app updateWindows];

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

                    pixel_t *const p = &pixel_buffer.pixels[x + y * pixel_buffer.width];
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

            [gl_context flushBuffer];
        }
    }

    return 0;
}