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
 *      Windowed mode gfx driver using gdi.
 *
 *      By Stefan Schimanski.
 *
 *      Dirty rectangles mechanism and hardware mouse cursor emulation
 *      by Eric Botcazou. 
 *
 *      See readme.txt for copyright information.
 */

#include "allegro.h" 
#include "allegro/aintern.h"
#include "allegro/aintwin.h"

/* function from asmlock.s */ 
extern void gfx_gdi_write_bank(void);
extern void gfx_gdi_unwrite_bank(void);

/* exported only for asmlock.s */
static void gfx_gdi_autolock(struct BITMAP* bmp);
static void gfx_gdi_unlock(struct BITMAP* bmp);
void (*ptr_gfx_gdi_autolock) (struct BITMAP* bmp) = gfx_gdi_autolock;
void (*ptr_gfx_gdi_unlock) (struct BITMAP* bmp) = gfx_gdi_unlock;
char *gdi_dirty_lines = NULL; /* used in WRITE_BANK() */


static struct BITMAP *gfx_gdi_init(int w, int h, int v_w, int v_h, int color_depth);
static void gfx_gdi_exit(struct BITMAP *b);
static void gfx_gdi_set_palette(AL_CONST struct RGB *p, int from, int to, int vsync);
static void gfx_gdi_vsync(void);
/* hardware mouse cursor emulation */
static int  gfx_gdi_set_mouse_sprite(struct BITMAP *sprite, int xfocus, int yfocus);
static int  gfx_gdi_show_mouse(struct BITMAP *bmp, int x, int y);
static void gfx_gdi_hide_mouse(void);
static void gfx_gdi_move_mouse(int x, int y);


GFX_DRIVER gfx_gdi =
{
   GFX_GDI,
   empty_string,
   empty_string,
   "GDI",
   gfx_gdi_init,
   gfx_gdi_exit,
   NULL,                        // AL_METHOD(int, scroll, (int x, int y)); 
   gfx_gdi_vsync,
   gfx_gdi_set_palette,
   NULL,                        // AL_METHOD(int, request_scroll, (int x, int y));
   NULL,                        // AL_METHOD(int poll_scroll, (void));
   NULL,                        // AL_METHOD(void, enable_triple_buffer, (void));
   NULL, NULL, NULL, NULL, NULL, NULL,
   gfx_gdi_set_mouse_sprite,
   gfx_gdi_show_mouse,
   gfx_gdi_hide_mouse,
   gfx_gdi_move_mouse,
   NULL,                        // AL_METHOD(void, drawing_mode, (void));
   NULL,                        // AL_METHOD(void, save_video_state, (void*));
   NULL,                        // AL_METHOD(void, restore_video_state, (void*));
   0, 0,                        // int w, h;                     /* physical (not virtual!) screen size */
   TRUE,                        // int linear;                   /* true if video memory is linear */
   0,                           // long bank_size;               /* bank size, in bytes */
   0,                           // long bank_gran;               /* bank granularity, in bytes */
   0,                           // long vid_mem;                 /* video memory size, in bytes */
   0,                           // long vid_phys_base;           /* physical address of video memory */
};


static void gdi_enter_sysmode(void);
static void gdi_exit_sysmode(void);
static void gdi_update_window(RECT *rect);


static struct WIN_GFX_DRIVER win_gfx_driver_gdi =
{
   NULL,                        // AL_METHOD(void, switch_in, (void));
   NULL,                        // AL_METHOD(void, switch_out, (void));
   gdi_enter_sysmode,
   gdi_exit_sysmode,
   NULL,                        // AL_METHOD(void, move, (int x, int y, int w, int h));
   NULL,                        // AL_METHOD(void, iconify, (void));
   gdi_update_window
};


static char *screen_surf;
static BITMAP *gdi_screen;
static int lock_nesting = 0;
static UINT render_timer = 0;
static int render_semaphore = FALSE;
static PALETTE palette;
static HANDLE vsync_event;
#define RENDER_DELAY (1000/70)

/* hardware mouse cursor emulation */
static int mouse_on = FALSE;
static int mouse_was_on = FALSE;
static BITMAP *mouse_sprite = NULL;
static BITMAP *mouse_frontbuffer = NULL;
static BITMAP *mouse_backbuffer = NULL;
static int mouse_xfocus, mouse_yfocus;
static int mouse_xpos, mouse_ypos;



/* gfx_gdi_set_mouse_sprite:
 */
static int gfx_gdi_set_mouse_sprite(struct BITMAP *sprite, int xfocus, int yfocus)
{
   if (mouse_sprite) {
      destroy_bitmap(mouse_sprite);
      mouse_sprite = NULL;

      destroy_bitmap(mouse_frontbuffer);
      mouse_frontbuffer = NULL;

      destroy_bitmap(mouse_backbuffer);
      mouse_backbuffer = NULL;
   }

   mouse_sprite = create_bitmap(sprite->w, sprite->h);
   blit(sprite, mouse_sprite, 0, 0, 0, 0, sprite->w, sprite->h);

   mouse_xfocus = xfocus;
   mouse_yfocus = yfocus;

   mouse_frontbuffer = create_bitmap(sprite->w, sprite->h);
   mouse_backbuffer = create_bitmap(sprite->w, sprite->h);

   return 0;
}



/* update_mouse_pointer:
 *  worker function that updates the mouse pointer
 */
static void update_mouse_pointer(int x, int y, int retrace)
{
   HDC hdc;

   /* put the screen contents located at the new position into the frontbuffer */
   blit(gdi_screen, mouse_frontbuffer, x, y, 0, 0,
        mouse_frontbuffer->w, mouse_frontbuffer->h);

   /* draw the mouse pointer onto the frontbuffer */
   draw_sprite(mouse_frontbuffer, mouse_sprite, 0, 0);

   hdc = GetDC(allegro_wnd);

   if (_color_depth == 8)
      set_palette_to_hdc(hdc, palette);

   if (retrace)
      /* restore the screen contents located at the old position */
      blit_to_hdc(mouse_backbuffer, hdc, 0, 0, mouse_xpos, mouse_ypos,
                  mouse_backbuffer->w, mouse_backbuffer->h);

   /* blit the mouse pointer onto the screen */
   blit_to_hdc(mouse_frontbuffer, hdc, 0, 0, x, y,
               mouse_frontbuffer->w, mouse_frontbuffer->h);

   ReleaseDC(allegro_wnd, hdc);

   /* save the screen contents located at the new position into the backbuffer */
   blit(gdi_screen, mouse_backbuffer, x, y, 0, 0,
        mouse_backbuffer->w, mouse_backbuffer->h);

   /* save the new position */
   mouse_xpos = x;
   mouse_ypos = y;
}



/* gfx_gdi_show_mouse:
 */
static int gfx_gdi_show_mouse(struct BITMAP *bmp, int x, int y)
{
   /* handle only the screen */
   if (bmp != gdi_screen)
      return -1;

   mouse_on = TRUE;

   x -= mouse_xfocus;
   y -= mouse_yfocus;

   update_mouse_pointer(x, y, FALSE);

   return 0;
} 



/* gfx_gdi_hide_mouse:
 */
static void gfx_gdi_hide_mouse(void)
{
   HDC hdc;

   if (!mouse_on)
      return; 

   hdc = GetDC(allegro_wnd);

   if (_color_depth == 8)
      set_palette_to_hdc(hdc, palette);

   /* restore the screen contents located at the old position */
   blit_to_hdc(mouse_backbuffer, hdc, 0, 0, mouse_xpos, mouse_ypos,
               mouse_backbuffer->w, mouse_backbuffer->h);

   ReleaseDC(allegro_wnd, hdc);

   mouse_on = FALSE;
}

 

/* gfx_gdi_move_mouse:
 */
static void gfx_gdi_move_mouse(int x, int y)
{
   if (!mouse_on)
      return;

   x -= mouse_xfocus;
   y -= mouse_yfocus;

   if ((mouse_xpos == x) && (mouse_ypos == y))
      return;

   update_mouse_pointer(x, y, TRUE);
} 



/* render_proc:
 *  timer proc that updates the window
 */
static void CALLBACK render_proc(HWND hwnd, UINT msg, UINT id_event, DWORD time)
{
   int top_line, bottom_line;
   HDC hdc = NULL;

   /* to prevent reentrant calls */
   if (render_semaphore)
      return;

   render_semaphore = TRUE;

   /* to prevent the drawing threads and the rendering proc
      from concurrently accessing the dirty lines array */
   _enter_gfx_critical();

   if (!gdi_screen) {
      _exit_gfx_critical();
      render_semaphore = FALSE;
      return;
   }

   /* pseudo dirty rectangles mechanism:
    *  at most only one GDI call is performed for each frame,
    *  a true dirty rectangles mechanism makes the demo game
    *  unplayable in 640x480 on my system
    */

   /* find the first dirty line */
   top_line = 0;

   while (!gdi_dirty_lines[top_line])
      top_line++;

   if (top_line < gfx_gdi.h) {
      /* find the last dirty line */
      bottom_line = gfx_gdi.h-1;

      while (!gdi_dirty_lines[bottom_line])
         bottom_line--;

      hdc = GetDC(allegro_wnd);

      if (_color_depth == 8)
         set_palette_to_hdc(hdc, palette);

      blit_to_hdc(gdi_screen, hdc, 0, top_line, 0, top_line,
                  gfx_gdi.w, bottom_line - top_line + 1);

      /* update mouse pointer if needed */
      if (mouse_on) {
         if ((mouse_ypos+mouse_sprite->h > top_line) && (mouse_ypos <= bottom_line)) {
            blit(gdi_screen, mouse_backbuffer, mouse_xpos, mouse_ypos, 0, 0,
                 mouse_backbuffer->w, mouse_backbuffer->h);

            update_mouse_pointer(mouse_xpos, mouse_ypos, TRUE);
         }
      }
      
      /* clean up the dirty lines */
      while (top_line <= bottom_line)
         gdi_dirty_lines[top_line++] = 0;

      ReleaseDC(allegro_wnd, hdc);
   }

   _exit_gfx_critical();

   /* simulate vertical retrace */
   PulseEvent(vsync_event);

   render_semaphore = FALSE;
}



/* gdi_enter_sysmode:
 */
static void gdi_enter_sysmode(void)
{
   /* hide the mouse pointer */
   if (mouse_on) {
      mouse_was_on = TRUE;
      gfx_gdi_hide_mouse();
      _TRACE("mouse pointer off\n");
   }
}



/* gdi_exit_sysmode:
 */
static void gdi_exit_sysmode(void)
{
   if (mouse_was_on) {
      mouse_on = TRUE;
      mouse_was_on = FALSE;
      _TRACE("mouse pointer on\n");
   }
}



/* gdi_update_window:
 *  updates the window
 */
static void gdi_update_window(RECT *rect)
{
   HDC hdc;

   _enter_gfx_critical();

   if (!gdi_screen) {
      _exit_gfx_critical();
      return;
   }

   hdc = GetDC(allegro_wnd);

   if (_color_depth == 8)
      set_palette_to_hdc(hdc, palette);

   blit_to_hdc(gdi_screen, hdc, rect->left, rect->top, rect->left, rect->top,
               rect->right - rect->left, rect->bottom - rect->top);

   ReleaseDC(allegro_wnd, hdc);

   _exit_gfx_critical();
}



/* gfx_gdi_lock:
 */
static void gfx_gdi_lock(struct BITMAP *bmp)
{
   /* to prevent the drawing threads and the rendering proc
      from concurrently accessing the dirty lines array */
   _enter_gfx_critical();

   /* arrange for drawing requests to pause when we are in the background */
   if (!app_foreground) {
      /* stop timer */
      KillTimer(allegro_wnd, render_timer);

      _exit_gfx_critical();

      if (GFX_CRITICAL_RELEASED)
         thread_switch_out();

      _enter_gfx_critical();

      /* restart timer */
      render_timer = SetTimer(allegro_wnd, 0, RENDER_DELAY, (TIMERPROC) render_proc);
   }

   lock_nesting++;
   bmp->id |= BMP_ID_LOCKED;
}



/* gfx_gdi_autolock:
 */
static void gfx_gdi_autolock(struct BITMAP *bmp)
{
   gfx_gdi_lock(bmp);
   bmp->id |= BMP_ID_AUTOLOCK;
}

 

/* gfx_gdi_unlock:
 */
static void gfx_gdi_unlock(struct BITMAP *bmp)
{
   if (lock_nesting > 0) {
      lock_nesting--;

      if (!lock_nesting)
         bmp->id &= ~BMP_ID_LOCKED;

      _exit_gfx_critical();
   }
}



/* gfx_gdi_init:
 */
static struct BITMAP *gfx_gdi_init(int w, int h, int v_w, int v_h, int color_depth)
{
   RECT win_size;

   /* virtual screen are not supported */
   if ((v_w!=0 && v_w!=w) || (v_h!=0 && v_h!=h))
      return NULL;
   
   _enter_critical();

   /* resize window */
   gfx_gdi.w = w;
   gfx_gdi.h = h;
   win_size.left = wnd_x = 32;
   win_size.right = 32 + w;
   win_size.top = wnd_y = 32;
   win_size.bottom = 32 + h;
   wnd_width = w;
   wnd_height = h;
   wnd_paint_back = TRUE;

   AdjustWindowRect(&win_size, GetWindowLong(allegro_wnd, GWL_STYLE), FALSE);
   MoveWindow(allegro_wnd, win_size.left, win_size.top, 
      win_size.right - win_size.left, win_size.bottom - win_size.top, TRUE);

   /* the last flag serves as end of loop delimiter */
   gdi_dirty_lines = calloc(h+1, sizeof(char));
   gdi_dirty_lines[h] = 1;

   /* set the default switching policy */
   wnd_windowed = TRUE;
   set_display_switch_mode(SWITCH_PAUSE);

   /* create the screen surface */
   screen_surf = malloc(w * h * BYTES_PER_PIXEL(color_depth));
   gdi_screen = _make_bitmap(w, h, (unsigned long) screen_surf, &gfx_gdi, color_depth, w * BYTES_PER_PIXEL(color_depth));
   gdi_screen->write_bank = gfx_gdi_write_bank; 
   _screen_vtable.acquire = gfx_gdi_lock;
   _screen_vtable.release = gfx_gdi_unlock;
   _screen_vtable.unwrite_bank = gfx_gdi_unwrite_bank; 

   /* connect to the system driver */
   win_gfx_driver = &win_gfx_driver_gdi;

   /* create render timer */
   vsync_event = CreateEvent(NULL, FALSE, FALSE, NULL);
   render_timer = SetTimer(allegro_wnd, 0, RENDER_DELAY, (TIMERPROC) render_proc);

   _exit_critical();

   return gdi_screen;
}



/* gfx_gdi_exit:
 */
static void gfx_gdi_exit(struct BITMAP *b)
{
   _enter_critical();

   _enter_gfx_critical();

   if (b)
      clear(b);

   /* stop timer */
   KillTimer(allegro_wnd, render_timer);
   CloseHandle(vsync_event);

   /* disconnect from the system driver */
   win_gfx_driver = NULL;

   /* destroy dirty lines array */   
   free(gdi_dirty_lines);
   gdi_dirty_lines = NULL;   

   /* destroy screen surface */
   free(screen_surf);
   gdi_screen = NULL;

   /* destroy mouse bitmaps */
   if (mouse_sprite) {
      destroy_bitmap(mouse_sprite);
      mouse_sprite = NULL;

      destroy_bitmap(mouse_frontbuffer);
      mouse_frontbuffer = NULL;

      destroy_bitmap(mouse_backbuffer);
      mouse_backbuffer = NULL;
   }

   _exit_gfx_critical();

   /* before restoring video mode, hide window */
   wnd_paint_back = FALSE;
   set_display_switch_mode(SWITCH_PAUSE);
   restore_window_style();
   SetWindowPos(allegro_wnd, HWND_TOP,
		-100, -100, 0, 0, SWP_SHOWWINDOW);

   _exit_critical();
}



/* gfx_gdi_set_palette:
 */
static void gfx_gdi_set_palette(AL_CONST struct RGB *p, int from, int to, int vsync)
{
   int c, line;

   for (c=from; c<=to; c++)
      palette[c] = p[c];

   /* invalidate the whole screen */
   gdi_dirty_lines[0] = gdi_dirty_lines[gfx_gdi.h-1] = 1;
}



/* gfx_gdi_vsync:
 */
static void gfx_gdi_vsync(void)
{
   WaitForSingleObject(vsync_event, INFINITE);
}
