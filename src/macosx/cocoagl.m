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

#include <OpenGL/gl.h>

#define PREFIX_I "al-osxgl INFO: "
#define PREFIX_W "al-osxgl WARNING: "

@class AllegroCocoaGLView;

static BITMAP *osx_gl_window_init(int, int, int, int, int);
static void osx_gl_window_exit(BITMAP *);

static BITMAP *osx_gl_full_init(int, int, int, int, int);
static void osx_gl_full_exit(BITMAP *);

static void gfx_cocoa_enable_acceleration(GFX_VTABLE *vtable);

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

    if (color_depth != 32 && color_depth != 16)
        ustrzcpy(allegro_error, ALLEGRO_ERROR_SIZE, get_config_text("Unsupported color depth"));

    BITMAP *displayed_video_bitmap = create_bitmap_ex(color_depth, w, h);

    driver->w = w;
    driver->h = h;
    driver->vid_mem = w * h * BYTES_PER_PIXEL(color_depth);

    gfx_cocoa_enable_acceleration(vtable);

    // Run on the main thread because we're manipulating the API
    runOnMainQueueWithoutDeadlocking(^{

    // setup REAL window
    NSRect rect = NSMakeRect(0, 0, w, h);
    NSUInteger styleMask = NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask;
    //if (is_fullscreen) {
      //  rect = [[NSScreen mainScreen] frame];
      //  styleMask = NSBorderlessWindowMask;
    //}

    osx_window = [[AllegroWindow alloc] initWithContentRect: rect
                                                  styleMask: styleMask
                                                    backing: NSBackingStoreBuffered
                                                      defer: NO
                                                     screen:[NSScreen mainScreen]];

    AllegroWindowDelegate *osx_window_delegate = [[AllegroWindowDelegate alloc] init];
    [osx_window setDelegate: (id<NSWindowDelegate>)osx_window_delegate];
    [osx_window setOneShot: YES];
    [osx_window setAcceptsMouseMovedEvents: YES];
    [osx_window setViewsNeedDisplay: NO];
    [osx_window setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];

    [osx_window center];
    if (is_fullscreen) {
        [osx_window  toggleFullScreen:nil];
//        [osx_window setLevel:NSMainMenuWindowLevel+1];
//        [osx_window setOpaque:YES];
//        [osx_window setHidesOnDeactivate:YES];
    }

    set_window_title(osx_window_title);

    AllegroCocoaGLView *osx_gl_view = [[[AllegroCocoaGLView alloc] initWithFrame: rect windowed:driver->windowed backing: displayed_video_bitmap] autorelease];
    [osx_window setContentView: osx_gl_view];

    [osx_window makeKeyAndOrderFront: nil];

    });

    osx_keyboard_focused(FALSE, 0);
    clear_keybuf();

    osx_skip_mouse_move = TRUE;
    osx_window_first_expose = TRUE;
    osx_gfx_mode = OSX_GFX_GL;

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
    runOnMainQueueWithoutDeadlocking(^{
        if (osx_window) {
            [[osx_window contentView] stopRendering];
            
            [osx_window close];
            osx_window = nil;
        }
    });

    osx_gfx_mode = OSX_GFX_NONE;
}


static void osx_gl_full_exit(BITMAP *bmp)
{
    _unix_lock_mutex(osx_skip_events_processing_mutex);
    osx_skip_events_processing = TRUE;
    _unix_unlock_mutex(osx_skip_events_processing_mutex);

    osx_gl_window_exit(bmp);
}

static void gfx_cocoa_enable_acceleration(GFX_VTABLE *vtable)
{
    gfx_capabilities |= (GFX_HW_VRAM_BLIT | GFX_HW_MEM_BLIT);
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
    *attrib++ = kCGLPFAOpenGLProfile;
    *attrib++ = kCGLOGLPVersion_Legacy;
    *attrib++ = NSOpenGLPFADoubleBuffer;
    *attrib++ = NSOpenGLPFAColorSize;
    *attrib++ = 16;

//   if (!windowed) {
//      *attrib++ = NSOpenGLPFAScreenMask;
//      *attrib++ = CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay);
//   }
   *attrib = 0;

   NSOpenGLPixelFormat *pf = [[NSOpenGLPixelFormat alloc] initWithAttributes: attribs];

   return pf;
}

static CVReturn display_link_render_callback(CVDisplayLinkRef displayLink, const CVTimeStamp* now, const CVTimeStamp* outputTime, CVOptionFlags flagsIn, CVOptionFlags* flagsOut, void* displayLinkContext)
{
    @autoreleasepool {
        [((AllegroCocoaGLView *)displayLinkContext) render];
    }
    return kCVReturnSuccess;
}

struct MyVertex {
    GLfloat x;
    GLfloat y;
};

@interface AllegroCocoaGLView: NSOpenGLView {

    BITMAP *_displayed_video_bitmap;

    int _gameWidth;
    int _gameHeight;
    int _colourDepth;

    GLuint _osx_screen_texture;
    int _osx_screen_color_depth;
    GLuint _osx_texture_format;
    GLenum _osx_texture_storage;

    CVDisplayLinkRef _displayLink;

    struct MyVertex _gl_VertexCoords[4];
    struct MyVertex _gl_TextureCoords[4];
}

@end

@implementation AllegroCocoaGLView

// Used to override default cursor
- (void)resetCursorRects
{
    [super resetCursorRects];
    [self addCursorRect: [self visibleRect]
                 cursor: osx_cursor];
    [osx_cursor setOnMouseEntered: YES];
}

/* Custom view: when created, select a suitable pixel format */
- (id) initWithFrame: (NSRect) frame windowed:(BOOL)windowed backing: (BITMAP *) bmp
{
    _displayed_video_bitmap = bmp;
    _gameWidth = bmp->w;
    _gameHeight = bmp->h;
    _colourDepth = bmp->vtable->color_depth;

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

- (void)renewGState
{   
    // Called whenever graphics state updated (such as window resize)
    
    // OpenGL rendering is not synchronous with other rendering on the OSX.
    // Therefore, call disableScreenUpdatesUntilFlush so the window server
    // doesn't render non-OpenGL content in the window asynchronously from
    // OpenGL content, which could cause flickering.  (non-OpenGL content
    // includes the title bar and drawing done by the app with other APIs)
    [[self window] disableScreenUpdatesUntilFlush];

    [super renewGState];
}

- (void)stopRendering
{
    CVDisplayLinkStop(_displayLink);

    CVDisplayLinkRelease(_displayLink);
    _displayLink = nil;
    
    CGLLockContext([[self openGLContext] CGLContextObj]);
    @try {
        [[self openGLContext] makeCurrentContext];
        
        _displayed_video_bitmap = nil;
        
        [self delete_osx_screen_texture];
        
        [NSOpenGLContext clearCurrentContext];
    } @finally {
        CGLUnlockContext([[self openGLContext] CGLContextObj]);
    }

    if (osx_mouse_tracking_rect != -1) {
      [self removeTrackingRect: osx_mouse_tracking_rect];
    }
}

- (void)dealloc
{
    [self stopRendering];

    [super dealloc];

}

- (void)delete_osx_screen_texture
{
    if (_osx_screen_texture != 0)
    {
        glDeleteTextures(1, &_osx_screen_texture);
        _osx_screen_texture = 0;
    }  
}

- (void) prepareOpenGL 
{
    [super prepareOpenGL];
    
    /* Print out OpenGL version info */
    TRACE(PREFIX_I "OpenGL Version: %s\n", (AL_CONST char*)glGetString(GL_VERSION));
    TRACE(PREFIX_I "Vendor: %s\n", (AL_CONST char*)glGetString(GL_VENDOR));
    TRACE(PREFIX_I "Renderer: %s\n", (AL_CONST char*)glGetString(GL_RENDERER));

    // enable vsync
    GLint swapInt = 1;
    [[self openGLContext] setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];

    [self osx_gl_setup];

    [[self openGLContext] flushBuffer];

    CVDisplayLinkCreateWithActiveCGDisplays(&_displayLink);

    CVDisplayLinkSetOutputCallback(_displayLink, &display_link_render_callback, self);

    // Select the display link most optimal for the current renderer of an OpenGL context
    CGLContextObj cglContext = [[self openGLContext] CGLContextObj];
    CGLPixelFormatObj cglPixelFormat = [[self pixelFormat] CGLPixelFormatObj];
    CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(_displayLink, cglContext, cglPixelFormat);

    CVDisplayLinkStart(_displayLink);
}

- (void) osx_gl_setup
{
    [self osx_gl_setup_arrays];

    glDisable(GL_DITHER);
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_FOG);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glPixelZoom(1.0,1.0);

    glShadeModel(GL_FLAT);
    glEnable(GL_TEXTURE_RECTANGLE_ARB);

    [self reshape];

    glEnableClientState(GL_VERTEX_ARRAY);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINES);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(2, GL_FLOAT, sizeof(struct MyVertex), (const GLvoid*)&_gl_VertexCoords[0]);
    glTexCoordPointer(2, GL_FLOAT, sizeof(struct MyVertex), (const GLvoid*)&_gl_TextureCoords[0]);

    glActiveTexture(GL_TEXTURE0);
    [self osx_gl_create_screen_texture];
}

- (void) osx_gl_create_screen_texture
{
    [self delete_osx_screen_texture];

    switch (_colourDepth) {
        case 16:
            _osx_texture_format = GL_RGB;
            _osx_texture_storage = GL_UNSIGNED_SHORT_5_6_5;
            break;
        case 32:
            _osx_texture_format = GL_RGBA;
            _osx_texture_storage = GL_UNSIGNED_BYTE;
            break;
        default:
            TRACE(PREFIX_I "unsupported color depth\n");
            return;
    }

    glGenTextures(1, &_osx_screen_texture);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, _osx_screen_texture);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, GL_RGB, _gameWidth, _gameHeight, 0, _osx_texture_format, _osx_texture_storage, NULL);
}

- (void) osx_gl_setup_arrays
{
    _gl_VertexCoords[0].x = 0;
    _gl_VertexCoords[0].y = 0;
    _gl_TextureCoords[0].x = 0;
    _gl_TextureCoords[0].y = _gameHeight;

    _gl_VertexCoords[1].x = _gameWidth;
    _gl_VertexCoords[1].y = 0;
    _gl_TextureCoords[1].x = _gameWidth;
    _gl_TextureCoords[1].y = _gameHeight;

    _gl_VertexCoords[2].x = 0;
    _gl_VertexCoords[2].y = _gameHeight;
    _gl_TextureCoords[2].x = 0;
    _gl_TextureCoords[2].y = 0;

    _gl_VertexCoords[3].x = _gameWidth;
    _gl_VertexCoords[3].y = _gameHeight;
    _gl_TextureCoords[3].x = _gameWidth;
    _gl_TextureCoords[3].y = 0;
}

- (void) drawRect: (NSRect) theRect
{
    [self render];
}


- (void) render
{
    [[self openGLContext] makeCurrentContext];
    
    CGLLockContext([[self openGLContext] CGLContextObj]);
    @try {
 
        if (!_displayed_video_bitmap) { return; }
        
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glTexSubImage2D(GL_TEXTURE_RECTANGLE_EXT, 0,
                        0, 0, _gameWidth, _gameHeight,
                        _osx_texture_format, _osx_texture_storage, _displayed_video_bitmap->line[0]);

        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        CGLFlushDrawable([[self openGLContext] CGLContextObj]);
    } @finally {
        CGLUnlockContext([[self openGLContext] CGLContextObj]);
    }
}


- (void) reshape
{
    [super reshape];

    if (osx_mouse_tracking_rect != -1) {
      [self removeTrackingRect: osx_mouse_tracking_rect];
    }

    osx_mouse_tracking_rect = [self addTrackingRect: self.frame
                                                     owner: NSApp
                                                  userData: nil
                                              assumeInside: YES];

    CGLLockContext([[self openGLContext] CGLContextObj]);
    @try {
        [[self openGLContext] makeCurrentContext];

        NSSize myNSWindowSize = [ self frame ].size;

        float aspect = ((float)_gameWidth) / _gameHeight;

        /* if 320x200 or 640x400 - non square pixels */
        if ( (_gameWidth == 320) && (_gameHeight == 200) ||
            (_gameWidth == 640) && (_gameHeight == 400) ) {
            aspect = 1.333333333333;
        }

        NSRect viewport = NSMakeRect((myNSWindowSize.width - myNSWindowSize.height * aspect)/2, 0, myNSWindowSize.height * aspect, myNSWindowSize.height);
        glViewport(viewport.origin.x, viewport.origin.y, viewport.size.width, viewport.size.height);
        glScissor(viewport.origin.x, viewport.origin.y, viewport.size.width, viewport.size.height);
        
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, _gameWidth - 1, 0, _gameHeight - 1, 0, 1);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    } @finally {
        CGLUnlockContext([[self openGLContext] CGLContextObj]);
    }

}

@end
