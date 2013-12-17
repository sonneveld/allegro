/*         ______   ___    ___ 
 *        /\  _  \ /\_ \  /\_ \ 
 *        \ \ \L\ \\//\ \ \//\ \      __     __   _ __   ___ 
 *         \ \  __ \ \ \ \  \ \ \   /'__`\ /'_ `\/\`'__\/ __`\
 *          \ \ \/\ \ \_\ \_ \_\ \_/\  __//\ \L\ \ \ \//\ \L\ \
 *           \ \_\ \_\/\____\/\____\ \____\ \____ \ \_\\ \____/
 *            \/_/\/_/\/____/\/____/\/____/\/___L\ \/_/ \/___/
 *                                           /\____/
 *                                           \_/__/
 *
 *      MacOS X quartz windowed gfx driver
 *
 *      By Angelo Mottola.
 *
 *      See readme.txt for copyright information.
 */


#include "allegro.h"
#include "allegro/internal/aintern.h"
#include "allegro/platform/aintosx.h"

#ifndef ALLEGRO_MACOSX
   #error something is wrong with the makefile
#endif

#define PREFIX_I "al-osxgl INFO: "
#define PREFIX_W "al-osxgl WARNING: "

static BITMAP *osx_gl_window_init(int, int, int, int, int);
static void osx_gl_window_exit(BITMAP *);

static BITMAP *osx_gl_full_init(int, int, int, int, int);
static void osx_gl_full_exit(BITMAP *);

static void gfx_cocoa_enable_acceleration(GFX_VTABLE *vtable);


// private bitmap that is actually rendered to
BITMAP* displayed_video_bitmap;

//other vars
static AllegroWindowDelegate *osx_window_delegate = NULL;
static AllegroCocoaGLView *osx_gl_view = NULL;
static NSOpenGLContext *osx_gl_context;

static NSOpenGLPixelFormat *init_pixel_format(int windowed);
static void osx_gl_setup(GFX_DRIVER*);
static void osx_gl_destroy();
static void osx_gl_create_screen_texture(int width, int height, int color_depth);
static void osx_gl_setup_arrays(int width, int height);

static GLuint osx_screen_texture = 0;
static int osx_screen_color_depth = 0;
static GLuint osx_texture_format = GL_RGBA;
static GLenum osx_texture_storage = GL_UNSIGNED_BYTE;


GFX_DRIVER gfx_cocoagl_window =
{
   GFX_COCOAGL_WINDOW,
   empty_string, 
   empty_string,
   "Cocoa GL window", 
   osx_gl_window_init,
   osx_gl_window_exit,
   NULL,                         /* AL_METHOD(int, scroll, (int x, int y)); */
   NULL,                         /* AL_METHOD(void, vsync, (void)); */
   NULL,                         /* AL_METHOD(void, set_palette, (AL_CONST struct RGB *p, int from, int to, int retracesync)); */
   NULL,                         /* AL_METHOD(int, request_scroll, (int x, int y)); */
   NULL,                         /* AL_METHOD(int, poll_scroll, (void)); */
   NULL,                         /* AL_METHOD(void, enable_triple_buffer, (void)); */
   NULL,                         /* AL_METHOD(struct BITMAP *, create_video_bitmap, (int width, int height)); */
   NULL,                         /* AL_METHOD(void, destroy_video_bitmap, (struct BITMAP *bitmap)); */
   NULL,                         /* AL_METHOD(int, show_video_bitmap, (BITMAP *bitmap)); */
   NULL,                         /* AL_METHOD(int, request_video_bitmap, (BITMAP *bitmap)); */
   NULL,                         /* AL_METHOD(BITMAP *, create_system_bitmap, (int width, int height)); */
   NULL,                         /* AL_METHOD(void, destroy_system_bitmap, (BITMAP *bitmap)); */
   osx_mouse_set_sprite,         /* AL_METHOD(int, set_mouse_sprite, (BITMAP *sprite, int xfocus, int yfocus)); */
   osx_mouse_show,               /* AL_METHOD(int, show_mouse, (BITMAP *bmp, int x, int y)); */
   osx_mouse_hide,               /* AL_METHOD(void, hide_mouse, (void)); */
   osx_mouse_move,               /* AL_METHOD(void, move_mouse, (int x, int y)); */
   NULL,                         /* AL_METHOD(void, drawing_mode, (void)); */
   NULL,                         /* AL_METHOD(void, save_video_state, (void)); */
   NULL,                         /* AL_METHOD(void, restore_video_state, (void)); */
   NULL,                         /* AL_METHOD(void, set_blender_mode, (int mode, int r, int g, int b, int a)); */
   NULL,                         /* AL_METHOD(int, fetch_mode_list, (void)); */
   0, 0,                         /* physical (not virtual!) screen size */
   TRUE,                         /* true if video memory is linear */
   0,                            /* bank size, in bytes */
   0,                            /* bank granularity, in bytes */
   0,                            /* video memory size, in bytes */
   0,                            /* physical address of video memory */
   TRUE                          /* true if driver runs windowed */
};

GFX_DRIVER gfx_cocoagl_full =
{
    GFX_COCOAGL_FULLSCREEN,
    empty_string,
    empty_string,
    "Cocoa GL full",
    osx_gl_full_init,
    osx_gl_full_exit,
    NULL,                         /* AL_METHOD(int, scroll, (int x, int y)); */
    NULL,                         /* AL_METHOD(void, vsync, (void)); */
    NULL,                         /* AL_METHOD(void, set_palette, (AL_CONST struct RGB *p, int from, int to, int retracesync)); */
    NULL,                         /* AL_METHOD(int, request_scroll, (int x, int y)); */
    NULL,                         /* AL_METHOD(int, poll_scroll, (void)); */
    NULL,                         /* AL_METHOD(void, enable_triple_buffer, (void)); */
    NULL,                         /* AL_METHOD(struct BITMAP *, create_video_bitmap, (int width, int height)); */
    NULL,                         /* AL_METHOD(void, destroy_video_bitmap, (struct BITMAP *bitmap)); */
    NULL,                         /* AL_METHOD(int, show_video_bitmap, (BITMAP *bitmap)); */
    NULL,                         /* AL_METHOD(int, request_video_bitmap, (BITMAP *bitmap)); */
    NULL,                         /* AL_METHOD(BITMAP *, create_system_bitmap, (int width, int height)); */
    NULL,                         /* AL_METHOD(void, destroy_system_bitmap, (BITMAP *bitmap)); */
    osx_mouse_set_sprite,         /* AL_METHOD(int, set_mouse_sprite, (BITMAP *sprite, int xfocus, int yfocus)); */
    osx_mouse_show,               /* AL_METHOD(int, show_mouse, (BITMAP *bmp, int x, int y)); */
    osx_mouse_hide,               /* AL_METHOD(void, hide_mouse, (void)); */
    osx_mouse_move,               /* AL_METHOD(void, move_mouse, (int x, int y)); */
    NULL,                         /* AL_METHOD(void, drawing_mode, (void)); */
    NULL,                         /* AL_METHOD(void, save_video_state, (void)); */
    NULL,                         /* AL_METHOD(void, restore_video_state, (void)); */
    NULL,                         /* AL_METHOD(void, set_blender_mode, (int mode, int r, int g, int b, int a)); */
    NULL,                         /* AL_METHOD(int, fetch_mode_list, (void)); */
    0, 0,                         /* physical (not virtual!) screen size */
    TRUE,                         /* true if video memory is linear */
    0,                            /* bank size, in bytes */
    0,                            /* bank granularity, in bytes */
    0,                            /* video memory size, in bytes */
    0,                            /* physical address of video memory */
    FALSE                         /* true if driver runs windowed */
};


static BITMAP *osx_gl_real_init(int w, int h, int v_w, int v_h, int color_depth, GFX_DRIVER * driver)
{
    bool is_fullscreen = driver->windowed == FALSE;

    GFX_VTABLE* vtable = _get_vtable(color_depth);

    _unix_lock_mutex(osx_event_mutex);

    if (color_depth != 32 && color_depth != 16)
        ustrzcpy(allegro_error, ALLEGRO_ERROR_SIZE, get_config_text("Unsupported color depth"));

    displayed_video_bitmap = create_bitmap_ex(color_depth, w, h);
    osx_screen_color_depth = color_depth;

    driver->w = w;
    driver->h = h;
    driver->vid_mem = w * h * BYTES_PER_PIXEL(color_depth);

    gfx_cocoa_enable_acceleration(vtable);

    // setup REAL window
	osx_window_mutex=_unix_create_mutex();
    _unix_lock_mutex(osx_window_mutex);

    NSRect rect = NSMakeRect(0, 0, w, h);
    NSUInteger styleMask = NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask;
    if (is_fullscreen) {
        rect = [[NSScreen mainScreen] frame];
        styleMask = NSBorderlessWindowMask;
    }

    osx_window = [[AllegroWindow alloc] initWithContentRect: rect
												  styleMask: styleMask
													backing: NSBackingStoreBuffered
													  defer: YES];

    osx_window_delegate = [[[AllegroWindowDelegate alloc] init] autorelease];
    [osx_window setDelegate: (id<NSWindowDelegate>)osx_window_delegate];
    [osx_window setOneShot: YES];
    [osx_window setAcceptsMouseMovedEvents: YES];
    [osx_window setViewsNeedDisplay: NO];
    [osx_window setReleasedWhenClosed: YES];
    if (is_fullscreen) {
        [osx_window setLevel:NSMainMenuWindowLevel+1];
        [osx_window setOpaque:YES];
        [osx_window setHidesOnDeactivate:YES];
    } else {
        [osx_window center];
    }

    set_window_title(osx_window_title);

    osx_gl_view = [[AllegroCocoaGLView alloc] initWithFrame: rect windowed:driver->windowed];
	[osx_window setContentView: osx_gl_view];

	[osx_window makeKeyAndOrderFront: nil];

    osx_gl_context = [[osx_gl_view openGLContext] retain];
	[osx_gl_context makeCurrentContext];

    // enable vsync
    GLint val = 1;
    [osx_gl_context setValues:&val forParameter:NSOpenGLCPSwapInterval];

    /* Print out OpenGL version info */
	TRACE(PREFIX_I "OpenGL Version: %s\n",
          (AL_CONST char*)glGetString(GL_VERSION));
	TRACE(PREFIX_I "Vendor: %s\n",
          (AL_CONST char*)glGetString(GL_VENDOR));
	TRACE(PREFIX_I "Renderer: %s\n",
          (AL_CONST char*)glGetString(GL_RENDERER));

    osx_keyboard_focused(FALSE, 0);
	clear_keybuf();

    osx_gl_setup(driver);

    [osx_gl_context flushBuffer];

    [NSOpenGLContext clearCurrentContext];

    if (!is_fullscreen) {
        osx_mouse_tracking_rect = [osx_gl_view addTrackingRect: rect
                                                         owner: NSApp
                                                      userData: nil
                                                  assumeInside: YES];
    }

    osx_gfx_mode = OSX_GFX_GL;
    osx_skip_mouse_move = TRUE;
    osx_window_first_expose = TRUE;

    _unix_unlock_mutex(osx_window_mutex);
    _unix_unlock_mutex(osx_event_mutex);

    return displayed_video_bitmap;
}

static BITMAP *osx_gl_window_init(int w, int h, int v_w, int v_h, int color_depth)
{
    BITMAP *bmp = osx_gl_real_init(w, h, v_w, v_h, color_depth, &gfx_cocoagl_window);
    
    return bmp;
}


static BITMAP *osx_gl_full_init(int w, int h, int v_w, int v_h, int color_depth)
{
    BITMAP *bmp = osx_gl_real_init(w, h, v_w, v_h, color_depth, &gfx_cocoagl_full);

    _unix_lock_mutex(osx_skip_events_processing_mutex);
    osx_skip_events_processing = FALSE;
    _unix_unlock_mutex(osx_skip_events_processing_mutex);

    return bmp;
}

static void osx_gl_window_exit(BITMAP *bmp)
{
    _unix_lock_mutex(osx_event_mutex);

    if (osx_window) {
        osx_gl_destroy();

        [osx_gl_context release];
        osx_gl_context = nil;
        
        [osx_gl_view release];
        osx_gl_view = nil;
        
        [osx_window close];
        osx_window = nil;
    }
//    destroy_bitmap(displayed_video_bitmap);
    displayed_video_bitmap = NULL;
    _unix_destroy_mutex(osx_window_mutex);
    osx_gfx_mode = OSX_GFX_NONE;

    _unix_unlock_mutex(osx_event_mutex);
}


static void osx_gl_full_exit(BITMAP *bmp)
{
    _unix_lock_mutex(osx_skip_events_processing_mutex);
    osx_skip_events_processing = TRUE;
    _unix_unlock_mutex(osx_skip_events_processing_mutex);

    osx_gl_window_exit(bmp);
}

void gfx_cocoa_enable_acceleration(GFX_VTABLE *vtable)
{  
    gfx_capabilities |= (GFX_HW_VRAM_BLIT | GFX_HW_MEM_BLIT);
}

struct MyVertex {
    GLfloat x;
    GLfloat y;
};
static struct MyVertex gl_VertexCoords[4];
static struct MyVertex gl_TextureCoords[4];

static void osx_gl_setup(GFX_DRIVER * driver)
{
    glEnable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_RECTANGLE_ARB);
//    glEnable(GL_TEXTURE_2D);
//    glDisable(GL_DEPTH_TEST);
//    glDisable(GL_LIGHTING);
//    glDisable(GL_BLEND);
//    glDisable(GL_SCISSOR_TEST);
    glShadeModel(GL_FLAT);
    
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    
    glViewport(0, 0, driver->w, driver->h);
    glScissor(0, 0, driver->w, driver->h);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnableClientState(GL_VERTEX_ARRAY);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINES);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(2, GL_FLOAT, sizeof(struct MyVertex), (const GLvoid*)&gl_VertexCoords[0]);
    glTexCoordPointer(2, GL_FLOAT, sizeof(struct MyVertex), (const GLvoid*)&gl_TextureCoords[0]);

	glActiveTexture(GL_TEXTURE0);

    osx_gl_create_screen_texture(driver->w, driver->h, osx_screen_color_depth);
    osx_gl_setup_arrays(driver->w, driver->h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(0, driver->w - 1, 0, driver->h - 1, 0, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

static void osx_gl_destroy()
{
    if (osx_screen_texture != 0)
    {
        glDeleteTextures(1, &osx_screen_texture);
    }
}

static void osx_gl_create_screen_texture(int width, int height, int color_depth)
{
    if (osx_screen_texture != 0)
    {
        glDeleteTextures(1, &osx_screen_texture);
    }
    
    glGenTextures(1, &osx_screen_texture);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, osx_screen_texture);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    if (color_depth == 16) {
        osx_texture_format = GL_RGB;
        osx_texture_storage = GL_UNSIGNED_SHORT_5_6_5;
    } else if (color_depth != 32) {
        TRACE(PREFIX_I "unsupported color depth\n");
    }

    glTexImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, GL_RGB, width, height, 0, osx_texture_format, osx_texture_storage, NULL);
}

static void osx_gl_setup_arrays(int width, int height)
{
    gl_VertexCoords[0].x = 0;
    gl_VertexCoords[0].y = 0;
    gl_TextureCoords[0].x = 0;
    gl_TextureCoords[0].y = height;

    gl_VertexCoords[1].x = width;
    gl_VertexCoords[1].y = 0;
    gl_TextureCoords[1].x = width;
    gl_TextureCoords[1].y = height;

    gl_VertexCoords[2].x = 0;
    gl_VertexCoords[2].y = height;
    gl_TextureCoords[2].x = 0;
    gl_TextureCoords[2].y = 0;
    
    gl_VertexCoords[3].x = width;
    gl_VertexCoords[3].y = height;
    gl_TextureCoords[3].x = width;
    gl_TextureCoords[3].y = 0;
}

void osx_gl_render()
{
    if (!gfx_driver) return;
    if (gfx_driver->id != GFX_COCOAGL_WINDOW && gfx_driver->id != GFX_COCOAGL_FULLSCREEN) return;

    _unix_lock_mutex(osx_window_mutex);
    if (osx_gl_context) {
        [osx_gl_context makeCurrentContext];
        glTexSubImage2D(GL_TEXTURE_RECTANGLE_EXT, 0,
                        0, 0, gfx_driver->w, gfx_driver->h,
                        osx_texture_format, osx_texture_storage, displayed_video_bitmap->line[0]);
        
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glFinish();
        [osx_gl_context flushBuffer];
        [NSOpenGLContext clearCurrentContext];
    }
    _unix_unlock_mutex(osx_window_mutex);
}

/* NSOpenGLPixelFormat *init_pixel_format(int windowed)
 *
 * Generate a pixel format. First try and get all the 'suggested' settings.
 * If this fails, just get the 'required' settings,
 * or nil if no format can be found
 */
#define MAX_ATTRIBUTES           64

static NSOpenGLPixelFormat *init_pixel_format(int windowed)
{
    NSOpenGLPixelFormatAttribute attribs[MAX_ATTRIBUTES], *attrib;
	attrib=attribs;
    *attrib++ = NSOpenGLPFADoubleBuffer;
    *attrib++ = NSOpenGLPFAAccelerated;
    *attrib++ = NSOpenGLPFAColorSize;
    *attrib++ = 16;

    /* Always request one of fullscreen or windowed */
	if (windowed) {
		*attrib++ = NSOpenGLPFAWindow;
		*attrib++ = NSOpenGLPFABackingStore;
	} else {
		*attrib++ = NSOpenGLPFAFullScreen;
		*attrib++ = NSOpenGLPFAScreenMask;
		*attrib++ = CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay);
	}
	*attrib = 0;

	NSOpenGLPixelFormat *pf = [[NSOpenGLPixelFormat alloc] initWithAttributes: attribs];

	return pf;
}

@implementation AllegroCocoaGLView

- (void)resetCursorRects
{
    [super resetCursorRects];
    [self addCursorRect: [self visibleRect]
                 cursor: osx_cursor];
    [osx_cursor setOnMouseEntered: YES];
}
/* Custom view: when created, select a suitable pixel format */
- (id) initWithFrame: (NSRect) frame windowed:(BOOL)windowed
{
	NSOpenGLPixelFormat* pf = init_pixel_format(windowed);
	if (pf) {
        self = [super initWithFrame:frame pixelFormat: pf];
        [pf release];
        return self;
	}
	else
	{
        TRACE(PREFIX_W "Unable to find suitable pixel format\n");
	}
	return nil;
}

-(BOOL)canBecomeKeyView
{
    return YES;
}
@end
