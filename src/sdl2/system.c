#include "allegro.h"
#include "allegro/internal/aintern.h"

#include "SDL.h"
#include <assert.h>

const int DEFAULT_DISPLAY_INDEX = 0;

static void (*_on_close_callback)(void) = 0;

static int mouse_x_mickeys = 0;
static int mouse_y_mickeys = 0;

// ----------------------------------------------------------------------------
// EVENTS!
// ----------------------------------------------------------------------------

static int allegro_scancode_from_sdl (int code) {
   switch (code) {
      case SDL_SCANCODE_A: return __allegro_KEY_A;
      case SDL_SCANCODE_B: return __allegro_KEY_B;
      case SDL_SCANCODE_C: return __allegro_KEY_C;
      case SDL_SCANCODE_D: return __allegro_KEY_D;
      case SDL_SCANCODE_E: return __allegro_KEY_E;
      case SDL_SCANCODE_F: return __allegro_KEY_F;
      case SDL_SCANCODE_G: return __allegro_KEY_G;
      case SDL_SCANCODE_H: return __allegro_KEY_H;
      case SDL_SCANCODE_I: return __allegro_KEY_I;
      case SDL_SCANCODE_J: return __allegro_KEY_J;
      case SDL_SCANCODE_K: return __allegro_KEY_K;
      case SDL_SCANCODE_L: return __allegro_KEY_L;
      case SDL_SCANCODE_M: return __allegro_KEY_M;
      case SDL_SCANCODE_N: return __allegro_KEY_N;
      case SDL_SCANCODE_O: return __allegro_KEY_O;
      case SDL_SCANCODE_P: return __allegro_KEY_P;
      case SDL_SCANCODE_Q: return __allegro_KEY_Q;
      case SDL_SCANCODE_R: return __allegro_KEY_R;
      case SDL_SCANCODE_S: return __allegro_KEY_S;
      case SDL_SCANCODE_T: return __allegro_KEY_T;
      case SDL_SCANCODE_U: return __allegro_KEY_U;
      case SDL_SCANCODE_V: return __allegro_KEY_V;
      case SDL_SCANCODE_W: return __allegro_KEY_W;
      case SDL_SCANCODE_X: return __allegro_KEY_X;
      case SDL_SCANCODE_Y: return __allegro_KEY_Y;
      case SDL_SCANCODE_Z: return __allegro_KEY_Z;
      case SDL_SCANCODE_0: return __allegro_KEY_0;
      case SDL_SCANCODE_1: return __allegro_KEY_1;
      case SDL_SCANCODE_2: return __allegro_KEY_2;
      case SDL_SCANCODE_3: return __allegro_KEY_3;
      case SDL_SCANCODE_4: return __allegro_KEY_4;
      case SDL_SCANCODE_5: return __allegro_KEY_5;
      case SDL_SCANCODE_6: return __allegro_KEY_6;
      case SDL_SCANCODE_7: return __allegro_KEY_7;
      case SDL_SCANCODE_8: return __allegro_KEY_8;
      case SDL_SCANCODE_9: return __allegro_KEY_9;
      case SDL_SCANCODE_KP_0: return __allegro_KEY_0_PAD;
      case SDL_SCANCODE_KP_1: return __allegro_KEY_1_PAD;
      case SDL_SCANCODE_KP_2: return __allegro_KEY_2_PAD;
      case SDL_SCANCODE_KP_3: return __allegro_KEY_3_PAD;
      case SDL_SCANCODE_KP_4: return __allegro_KEY_4_PAD;
      case SDL_SCANCODE_KP_5: return __allegro_KEY_5_PAD;
      case SDL_SCANCODE_KP_6: return __allegro_KEY_6_PAD;
      case SDL_SCANCODE_KP_7: return __allegro_KEY_7_PAD;
      case SDL_SCANCODE_KP_8: return __allegro_KEY_8_PAD;
      case SDL_SCANCODE_KP_9: return __allegro_KEY_9_PAD;
      case SDL_SCANCODE_F1: return __allegro_KEY_F1;
      case SDL_SCANCODE_F2: return __allegro_KEY_F2;
      case SDL_SCANCODE_F3: return __allegro_KEY_F3;
      case SDL_SCANCODE_F4: return __allegro_KEY_F4;
      case SDL_SCANCODE_F5: return __allegro_KEY_F5;
      case SDL_SCANCODE_F6: return __allegro_KEY_F6;
      case SDL_SCANCODE_F7: return __allegro_KEY_F7;
      case SDL_SCANCODE_F8: return __allegro_KEY_F8;
      case SDL_SCANCODE_F9: return __allegro_KEY_F9;
      case SDL_SCANCODE_F10: return __allegro_KEY_F10;
      case SDL_SCANCODE_F11: return __allegro_KEY_F11;
      case SDL_SCANCODE_F12: return __allegro_KEY_F12;
      case SDL_SCANCODE_ESCAPE: return __allegro_KEY_ESC;
      case SDL_SCANCODE_MINUS: return __allegro_KEY_MINUS;
      case SDL_SCANCODE_EQUALS: return __allegro_KEY_EQUALS;
      case SDL_SCANCODE_BACKSPACE: return __allegro_KEY_BACKSPACE;
      case SDL_SCANCODE_TAB: return __allegro_KEY_TAB;
      case SDL_SCANCODE_LEFTBRACKET: return __allegro_KEY_OPENBRACE;
      case SDL_SCANCODE_RIGHTBRACKET: return __allegro_KEY_CLOSEBRACE;
      case SDL_SCANCODE_RETURN: return __allegro_KEY_ENTER;
      case SDL_SCANCODE_APOSTROPHE: return __allegro_KEY_QUOTE;
      case SDL_SCANCODE_BACKSLASH: return __allegro_KEY_BACKSLASH;
      case SDL_SCANCODE_COMMA: return __allegro_KEY_COMMA;
      case SDL_SCANCODE_PERIOD: return __allegro_KEY_STOP;
      case SDL_SCANCODE_SLASH: return __allegro_KEY_SLASH;
      case SDL_SCANCODE_SPACE: return __allegro_KEY_SPACE;
      case SDL_SCANCODE_INSERT: return __allegro_KEY_INSERT;
      case SDL_SCANCODE_DELETE: return __allegro_KEY_DEL;
      case SDL_SCANCODE_HOME: return __allegro_KEY_HOME;
      case SDL_SCANCODE_END: return __allegro_KEY_END;
      case SDL_SCANCODE_PAGEUP: return __allegro_KEY_PGUP;
      case SDL_SCANCODE_PAGEDOWN: return __allegro_KEY_PGDN;
      case SDL_SCANCODE_LEFT: return __allegro_KEY_LEFT;
      case SDL_SCANCODE_RIGHT: return __allegro_KEY_RIGHT;
      case SDL_SCANCODE_UP: return __allegro_KEY_UP;
      case SDL_SCANCODE_DOWN: return __allegro_KEY_DOWN;
      case SDL_SCANCODE_KP_DIVIDE: return __allegro_KEY_SLASH_PAD;
      case SDL_SCANCODE_KP_MULTIPLY: return __allegro_KEY_ASTERISK;
      case SDL_SCANCODE_KP_MINUS: return __allegro_KEY_MINUS_PAD;
      case SDL_SCANCODE_KP_PLUS: return __allegro_KEY_PLUS_PAD;
      case SDL_SCANCODE_KP_BACKSPACE: return __allegro_KEY_DEL_PAD;
      case SDL_SCANCODE_KP_ENTER: return __allegro_KEY_ENTER_PAD;
      case SDL_SCANCODE_PRINTSCREEN: return __allegro_KEY_PRTSCR;
      case SDL_SCANCODE_PAUSE: return __allegro_KEY_PAUSE;
      case SDL_SCANCODE_KP_EQUALS: return __allegro_KEY_EQUALS_PAD;
      case SDL_SCANCODE_GRAVE: return __allegro_KEY_BACKQUOTE;
      case SDL_SCANCODE_SEMICOLON: return __allegro_KEY_SEMICOLON;
      case SDL_SCANCODE_LSHIFT: return __allegro_KEY_LSHIFT;
      case SDL_SCANCODE_RSHIFT: return __allegro_KEY_RSHIFT;
      case SDL_SCANCODE_LCTRL: return __allegro_KEY_LCONTROL;
      case SDL_SCANCODE_RCTRL: return __allegro_KEY_RCONTROL;
      case SDL_SCANCODE_LALT: return __allegro_KEY_ALT;
      case SDL_SCANCODE_RALT: return __allegro_KEY_ALT;
#ifdef ALLEGRO_MACOSX
      case SDL_SCANCODE_LGUI: return __allegro_KEY_COMMAND;
      case SDL_SCANCODE_RGUI: return __allegro_KEY_COMMAND;
#else
      case SDL_SCANCODE_LGUI: return __allegro_KEY_LWIN;
      case SDL_SCANCODE_RGUI: return __allegro_KEY_RWIN;
#endif
      case SDL_SCANCODE_APPLICATION: return __allegro_KEY_MENU;
      case SDL_SCANCODE_SCROLLLOCK: return __allegro_KEY_SCRLOCK;
      case SDL_SCANCODE_NUMLOCKCLEAR: return __allegro_KEY_NUMLOCK;
      case SDL_SCANCODE_CAPSLOCK: return __allegro_KEY_CAPSLOCK;
      default: return -1;
   }
}

static int allegro_mod_flags_from_sdl(int code) {
   // AGS only cares about scroll-lock, numlock, capslock, alt, ctrl
    
   switch (code) {
      case  SDL_SCANCODE_LSHIFT:  return KB_SHIFT_FLAG;
      case  SDL_SCANCODE_RSHIFT:  return KB_SHIFT_FLAG;
         
      case  SDL_SCANCODE_LCTRL:  return KB_CTRL_FLAG;
      case  SDL_SCANCODE_RCTRL:  return KB_CTRL_FLAG;
         
      case  SDL_SCANCODE_LALT:  return KB_ALT_FLAG;
      case  SDL_SCANCODE_RALT:  return KB_ALT_FLAG;
#ifdef ALLEGRO_MACOSX
      case  SDL_SCANCODE_LGUI:  return KB_COMMAND_FLAG;
      case  SDL_SCANCODE_RGUI:  return KB_COMMAND_FLAG;
#else
      case SDL_SCANCODE_LGUI:  return KB_LWIN_FLAG;
      case SDL_SCANCODE_RGUI:  return KB_RWIN_FLAG;
#endif
         
      case  SDL_SCANCODE_APPLICATION:  return KB_MENU_FLAG;
      case  SDL_SCANCODE_SCROLLLOCK:  return KB_SCROLOCK_FLAG;
      case  SDL_SCANCODE_NUMLOCKCLEAR:  return KB_NUMLOCK_FLAG;
      case  SDL_SCANCODE_CAPSLOCK:  return KB_CAPSLOCK_FLAG;
   }
   return 0;
}

static void on_sdl_key_up_down(SDL_KeyboardEvent *event) {
   const int scancode = allegro_scancode_from_sdl(event->keysym.scancode);
    
   if (scancode <= 0) { return; }

   int keycode = event->keysym.sym & 0x7f;
    
   const int keyflags = allegro_mod_flags_from_sdl (event->keysym.scancode);
   if (keyflags) {
      if (event->state == SDL_PRESSED) {
         _key_shifts &= ~keyflags;
      } else {
         _key_shifts |= keyflags;
      }
   }
    
   if (event->state == SDL_PRESSED) {
      _handle_key_press(keycode, scancode);
   } else {
      _handle_key_release(scancode);
   }
}

void on_sdl_mouse_motion(SDL_MouseMotionEvent *event) {
   _mouse_x = event->x;
   _mouse_y = event->y;
   mouse_x_mickeys += event->xrel;
   mouse_y_mickeys += event->yrel;
   _handle_mouse_input(); 
}

static int sdl_button_to_allegro_bit(int button)
{
   switch (button) {
      case SDL_BUTTON_LEFT: return 0x1;
      case SDL_BUTTON_RIGHT: return 0x2;
      case SDL_BUTTON_MIDDLE: return 0x4;
      case SDL_BUTTON_X1: return 0x8;
      case SDL_BUTTON_X2: return 0x10;
   }
   return 0x0;
}

void on_sdl_mouse_button(SDL_MouseButtonEvent *event) 
{
   _mouse_x = event->x;
   _mouse_y = event->y;

   if (event->type == SDL_MOUSEBUTTONDOWN) {
      _mouse_b |= sdl_button_to_allegro_bit(event->button);
   } else {
      _mouse_b &= ~sdl_button_to_allegro_bit(event->button);
   }

   _handle_mouse_input(); 
}

void process_sdl2_events (void) {
   SDL_Event event;
   while (SDL_PollEvent(&event)) {
      switch (event.type) {
         case SDL_KEYDOWN:
         case SDL_KEYUP:
            on_sdl_key_up_down(&event.key);
            break;
         case SDL_MOUSEMOTION:
            on_sdl_mouse_motion(&event.motion);
            break;
         case SDL_MOUSEBUTTONDOWN:
         case SDL_MOUSEBUTTONUP:
            on_sdl_mouse_button(&event.button);
            break;
         case SDL_QUIT:
            if (_on_close_callback) {
               _on_close_callback();
            }
            break;
      }
   }
}


// ----------------------------------------------------------------------------
// SYSTEM
// ----------------------------------------------------------------------------

static int trace_handler(AL_CONST char *msg) {
   SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "%s", msg);
   return 0;
}

static int sdl2_sys_init(void)
{
   SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
   
   if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_TIMER|SDL_INIT_EVENTS) != 0) {
      SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
      return -1;
   }

   register_trace_handler(trace_handler);
   
   os_type = OSTYPE_SDL2;
   os_version = -1; 
   os_revision = -1;
   os_multitasking = TRUE;
   
   return 0;
}

static void sdl2_sys_exit(void)
{
   SDL_Quit();
}

int _unix_find_resource(char *dest, AL_CONST char *resource, int size);

static int sdl2_sys_find_resource(char *dest, AL_CONST char *resource, int size)
{
   char buf[4096], tmp[4096];
   
   char *base_path = SDL_GetBasePath();
   if (base_path) {
      append_filename(buf, uconvert_ascii(base_path, tmp), resource, sizeof(buf));
      SDL_free(base_path);
      if (exists(buf)) {
         ustrzcpy(dest, size, buf);
	      return 0;
      }
   }

   /* char *pref_path = SDL_GetPrefPath();
   if (pref_path) {
      append_filename(buf, uconvert_ascii(pref_path, tmp), resource, sizeof(buf));
      SDL_free(pref_path);
      if (exists(buf)) {
         ustrzcpy(dest, size, buf);
	      return 0;
      }
   } */

   return _unix_find_resource(dest, resource, size);
}


static void sdl2_gfx_set_window_title(AL_CONST char *title);

static int sdl2_sys_set_close_button_callback(void (*proc)(void)) {
   _on_close_callback = proc;
   return 0;
}

static void sdl2_sys_message(AL_CONST char *msg) {
   SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", msg, NULL);
}
 
static int sdl2_sys_get_desktop_resolution(int *width, int *height) {
   SDL_Rect r;
   if (SDL_GetDisplayBounds(DEFAULT_DISPLAY_INDEX, &r) != 0) {
      SDL_Log("SDL_GetDisplayBounds failed: %s", SDL_GetError());
      return -1;
   }
   *width = r.w;
   *height = r.h;
   return 0;
}

void *sdl2_create_mutex(void) {
   void *result = SDL_CreateMutex();
   if (result == NULL) {
      SDL_Log("SDL_CreateMutex failed: %s", SDL_GetError());
   }
   return result;
}

void sdl2_destroy_mutex(void *handle) {
   SDL_DestroyMutex(handle);
}

void sdl2_lock_mutex(void *handle) {
   SDL_LockMutex(handle);
}

void sdl2_unlock_mutex(void *handle) {
   SDL_UnlockMutex(handle);
}

SYSTEM_DRIVER system_sdl2 =
{
   .id = SYSTEM_SDL2,
   .name = empty_string,
   .desc = empty_string,
   .ascii_name = "MacOS X",

   .init = sdl2_sys_init,
   .exit = sdl2_sys_exit,

   .find_resource = sdl2_sys_find_resource,

   .set_window_title = sdl2_gfx_set_window_title,
   .set_close_button_callback = sdl2_sys_set_close_button_callback,

   .message = sdl2_sys_message,

   .get_desktop_resolution = sdl2_sys_get_desktop_resolution,

   .create_mutex = sdl2_create_mutex,
   .destroy_mutex = sdl2_destroy_mutex,
   .lock_mutex = sdl2_lock_mutex,
   .unlock_mutex = sdl2_unlock_mutex,
};


// ----------------------------------------------------------------------------
// TIMER
// ----------------------------------------------------------------------------

#define TIMER_TO_MSEC(x)  ((long)( ((x) / 1193.1810) + 0.5))
// #define MSEC_TO_TIMER(x)  ((long)((x) * (TIMERS_PER_SECOND / 1000.)))

static SDL_TimerID my_timer_id;

Uint32 sdl2_timerint(Uint32 interval_ms, void *param)
{
   int interval_timer = _handle_timer_tick(MSEC_TO_TIMER(interval_ms));
   int result = TIMER_TO_MSEC(interval_timer);
   if (result <= 0) { result = 1; }  // has to be at least 1 or the sdl2 timer will stop
   return result;
}

static int sdl2_timer_init(void)
{
   my_timer_id = SDL_AddTimer(25, sdl2_timerint, NULL);
   return 0;
}

static void sdl2_timer_exit(void)
{
   SDL_RemoveTimer(my_timer_id);
}

void sdl2_rest(unsigned int ms, void (*callback) (void)) {
   // callback ignored
   assert(callback == NULL);
   SDL_Delay(ms);
}

TIMER_DRIVER timer_sdl2 =
{
   TIMER_SDL2,
   empty_string,
   empty_string,
   "SDL2 timer",
   sdl2_timer_init,
   sdl2_timer_exit,
   NULL, NULL,		/* install_int, remove_int */
   NULL, NULL,		/* install_param_int, remove_param_int */
   NULL, NULL,		/* can_simulate_retrace, simulate_retrace */
   sdl2_rest		/* rest */
};


// ----------------------------------------------------------------------------
// AUDIO
// ----------------------------------------------------------------------------

static SDL_AudioDeviceID dev;

static int digi_sdl2_detect(int input)
{
   if (input) {
      return 0;
   }
   return 1;
}

void MyAudioCallback(void*  userdata, Uint8* stream, int len) 
{
   _mix_some_samples((uintptr_t)stream, 0, TRUE);
}

static int digi_sdl2_init(int input, int voices) {
   if (input) {
      ustrzcpy(allegro_error, ALLEGRO_ERROR_SIZE, get_config_text("Input is not supported"));
      return -1;
   }

   digi_sdl2.voices = voices;

   SDL_AudioSpec want, have;

   SDL_memset(&want, 0, sizeof(want)); /* or SDL_zero(want) */
   want.freq = 48000;
   want.format = AUDIO_S16SYS;
   want.channels = 2;
   want.samples = 2048;  // number of frames, not samples
   want.callback = MyAudioCallback; 

   dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);

   if (_mixer_init(have.samples * have.channels, have.freq, TRUE, 1, &digi_sdl2.voices) != 0) {
      return -1;
   }

   SDL_PauseAudioDevice(dev, 0);

   return 0;
}

static void digi_sdl2_exit(int input) {
   SDL_CloseAudioDevice(dev);
}


DIGI_DRIVER digi_sdl2 =
{
   0,
   empty_string,
   empty_string,
   empty_string,
   0,              // available voices
   0,              // voice number offset
   MIXER_MAX_SFX,  // maximum voices we can support
   MIXER_DEF_SFX,  // default number of voices to use

   /* setup routines */
   digi_sdl2_detect,
   digi_sdl2_init,
   digi_sdl2_exit,
   NULL, // digi_sdl2_set_mixer_volume,
   NULL, // digi_sdl2_get_mixer_volume,

   /* audiostream locking functions */
   NULL,  // AL_METHOD(void *, lock_voice, (int voice, int start, int end));
   NULL,  // AL_METHOD(void, unlock_voice, (int voice));
   NULL, // digi_sdl2_buffer_size,

   /* voice control functions */
   _mixer_init_voice,
   _mixer_release_voice,
   _mixer_start_voice,
   _mixer_stop_voice,
   _mixer_loop_voice,

   /* position control functions */
   _mixer_get_position,
   _mixer_set_position,

   /* volume control functions */
   _mixer_get_volume,
   _mixer_set_volume,
   _mixer_ramp_volume,
   _mixer_stop_volume_ramp,

   /* pitch control functions */
   _mixer_get_frequency,
   _mixer_set_frequency,
   _mixer_sweep_frequency,
   _mixer_stop_frequency_sweep,

   /* pan control functions */
   _mixer_get_pan,
   _mixer_set_pan,
   _mixer_sweep_pan,
   _mixer_stop_pan_sweep,

   /* effect control functions */
   _mixer_set_echo,
   _mixer_set_tremolo,
   _mixer_set_vibrato,

   /* input functions */
   0,     // int rec_cap_bits;
   0,     // int rec_cap_stereo;
   NULL,  // AL_METHOD(int,  rec_cap_rate, (int bits, int stereo));
   NULL,  // AL_METHOD(int,  rec_cap_parm, (int rate, int bits, int stereo));
   NULL,  // AL_METHOD(int,  rec_source, (int source));
   NULL,  // AL_METHOD(int,  rec_start, (int rate, int bits, int stereo));
   NULL,  // AL_METHOD(void, rec_stop, (void));
   NULL   // AL_METHOD(int,  rec_read, (void *buf));
};


// ----------------------------------------------------------------------------
// GRAPHICS
// ----------------------------------------------------------------------------

static BITMAP * displayed_video_bitmap;
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *screenTex = NULL;
static const char * window_title = "AGS";

static BITMAP *gfx_sdl2_init_driver(GFX_DRIVER *drv, int w, int h, int v_w, int v_h, int color_depth) {

   if (color_depth != 32) {
      ustrzcpy(allegro_error, ALLEGRO_ERROR_SIZE, get_config_text("Unsupported color depth"));
      return NULL;
   }

   displayed_video_bitmap = create_bitmap_ex(color_depth, w, h);

   Uint32 flags = SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE;
   if (!drv->windowed) {
      flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
   }

   window = SDL_CreateWindow(
        window_title,                  // window title
        SDL_WINDOWPOS_CENTERED_DISPLAY(DEFAULT_DISPLAY_INDEX),           // initial x position
        SDL_WINDOWPOS_CENTERED_DISPLAY(DEFAULT_DISPLAY_INDEX),           // initial y position
        w,                               // width, in pixels
        h,                               // height, in pixels
        flags                 // flags 
    );

   renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);

   SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");  // make the scaled rendering look smoother.
   SDL_RenderSetLogicalSize(renderer, w, h);

   screenTex = SDL_CreateTexture( renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, w, h );

   drv->w = w;
   drv->h = h;

   return displayed_video_bitmap;
}


static BITMAP *gfx_sdl2_init_window(int w, int h, int v_w, int v_h, int color_depth) {
   return gfx_sdl2_init_driver(&gfx_sdl2_window, w, h, v_w, v_h, color_depth);
}

static BITMAP *gfx_sdl2_init_fullscreen(int w, int h, int v_w, int v_h, int color_depth) {
   return gfx_sdl2_init_driver(&gfx_sdl2_fullscreen, w, h, v_w, v_h, color_depth);
}


static void gfx_sdl2_exit(BITMAP *bmp) {
   SDL_DestroyTexture(screenTex);
   SDL_DestroyRenderer(renderer);
   SDL_DestroyWindow(window);
}

static void sdl2_gfx_set_window_title(AL_CONST char *title) {
   window_title = title;
   if (window) {
      SDL_SetWindowTitle(window, window_title);
   }
}

void sdl2_present_screen() {
   SDL_RenderClear(renderer);

   /*unsigned char * pixels;
   int pitch;

   SDL_LockTexture(screenTex, NULL, (void **)&pixels, &pitch);

   for (int i = 0; i < displayed_video_bitmap->h; i++) {
      memcpy(pixels, displayed_video_bitmap->line[i], pitch);
      pixels += pitch;
   }

   SDL_UnlockTexture(screenTex); */

   SDL_UpdateTexture(screenTex, NULL, displayed_video_bitmap->line[0], displayed_video_bitmap->w * sizeof (Uint32));

   SDL_RenderCopy(renderer, screenTex, NULL, NULL);
   SDL_RenderPresent(renderer);
}

static int gfx_sdl2_show_mouse(struct BITMAP *bmp, int x, int y) {
   SDL_ShowCursor(SDL_ENABLE);
   return -1; // show cursor, but "fail" because we're not supporting hardware cursors
}

static void gfx_sdl2_hide_mouse() {
   SDL_ShowCursor(SDL_DISABLE);
}

static GFX_MODE_LIST *gfx_sdl2_window_fetch_mode_list (void) {
   SDL_Rect rect;
#if SDL_VERSION_ATLEAST(2, 0, 5)
   int result = SDL_GetDisplayUsableBounds(DEFAULT_DISPLAY_INDEX, &rect);
#else
   int result = SDL_GetDisplayBounds(DEFAULT_DISPLAY_INDEX, &rect);
#endif
   if (result != 0) { return NULL; }

   GFX_MODE_LIST *mode_list = _AL_MALLOC(sizeof(GFX_MODE_LIST));
   mode_list->num_modes = 1;
   mode_list->mode = _AL_MALLOC(sizeof(GFX_MODE)*2);
   mode_list->mode[0] = (GFX_MODE){
      .width = rect.w,
      .height = rect.h,
      .bpp = 32
   };
   mode_list->mode[1] = (GFX_MODE){0};
   return mode_list;
}

static GFX_MODE_LIST *gfx_sdl2_fullscreen_fetch_mode_list (void) {
   SDL_Rect rect;
   int result = SDL_GetDisplayBounds(DEFAULT_DISPLAY_INDEX, &rect);
   if (result != 0) { return NULL; }

   GFX_MODE_LIST *mode_list = _AL_MALLOC(sizeof(GFX_MODE_LIST));
   mode_list->num_modes = 1;
   mode_list->mode = _AL_MALLOC(sizeof(GFX_MODE)*2);
   mode_list->mode[0] = (GFX_MODE){
      .width = rect.w,
      .height = rect.h,
      .bpp = 32
   };
   mode_list->mode[1] = (GFX_MODE){0};
   return mode_list;
}

GFX_DRIVER gfx_sdl2_window =
{
   .id = GFX_SDL2_WINDOW,
   .name = empty_string,
   .desc = empty_string,
   .ascii_name = "SDL2 Windowed",
   .init = gfx_sdl2_init_window,
   .exit = gfx_sdl2_exit,
   //.vsync = gfx_sdl2_vsync,             
   // .create_video_bitmap = NULL, //gfx_sdl2_create_video_bitmap,
   // .destroy_video_bitmap = NULL, // gfx_sdl2_destroy_video_bitmap,
   // .set_mouse_sprite = NULL, // gfx_sdl2_set_mouse_sprite,
   .show_mouse = gfx_sdl2_show_mouse, // gfx_sdl2_show_mouse,
   .hide_mouse = gfx_sdl2_hide_mouse, // gfx_sdl2_hide_mouse,
   // .move_mouse = NULL, // gfx_sdl2_move_mouse,
   .fetch_mode_list = gfx_sdl2_window_fetch_mode_list,
   .linear = TRUE,
   .windowed = TRUE                   
};

GFX_DRIVER gfx_sdl2_fullscreen =
{
   .id = GFX_SDL2_FULLSCREEN,
   .name = empty_string,
   .desc = empty_string,
   .ascii_name = "SDL2 Fullscreen",
   .init = gfx_sdl2_init_fullscreen,
   .exit = gfx_sdl2_exit,
   //.vsync = gfx_sdl2_vsync,             
   // .create_video_bitmap = NULL, //gfx_sdl2_create_video_bitmap,
   // .destroy_video_bitmap = NULL, // gfx_sdl2_destroy_video_bitmap,
   // .set_mouse_sprite = NULL, // gfx_sdl2_set_mouse_sprite,
   .show_mouse = gfx_sdl2_show_mouse, // gfx_sdl2_show_mouse,
   .hide_mouse = gfx_sdl2_hide_mouse, // gfx_sdl2_hide_mouse,
   // .move_mouse = NULL, // gfx_sdl2_move_mouse,
   .fetch_mode_list = gfx_sdl2_fullscreen_fetch_mode_list,
   .linear = TRUE,
   .windowed = FALSE                   
};


// ----------------------------------------------------------------------------
// MOUSE
// ----------------------------------------------------------------------------

static int sdl2_mouse_init(void) {
   return 0;
}

static void sdl2_mouse_exit(void) {
}

static void sdl2_mouse_poll(void) {
   process_sdl2_events();
}

static void sdl2_mouse_position(int x, int y) {
   SDL_WarpMouseInWindow(window, x, y);
}

static void sdl2_mouse_get_mickeys (int *mickeyx, int *mickeyy)
{
   *mickeyx = mouse_x_mickeys;
   *mickeyy = mouse_y_mickeys;
   mouse_x_mickeys = 0;
   mouse_y_mickeys = 0;
}

MOUSE_DRIVER mouse_sdl2 = {
   .id = MOUSE_SDL2,
   .name = empty_string,
   .desc = empty_string,
   .ascii_name = "SDL2 mouse",
   .init = sdl2_mouse_init,
   .exit = sdl2_mouse_exit,
   .poll = sdl2_mouse_poll,       // AL_METHOD(void, poll, (void));
   //.timer_poll = NULL,       // AL_METHOD(void, timer_poll, (void));
   // .position = sdl2_mouse_position,
   // .set_range = sdl2_mouse_set_range,
   // .set_speed = NULL,       // AL_METHOD(void, set_speed, (int xspeed, int yspeed));
   .get_mickeys = sdl2_mouse_get_mickeys,
   // .analyse_data = NULL,       // AL_METHOD(int,  analyse_data, (AL_CONST char *buffer, int size));
   // .enable_hardware_cursor = sdl2_enable_hardware_cursor, 
   // .select_system_cursor = sdl2_select_system_cursor
};


// ----------------------------------------------------------------------------
// KEYBOARD
// ----------------------------------------------------------------------------

static int sdl2_keyboard_init(void) {
   return 0;
}

static void sdl2_keyboard_exit(void) {
}

static void sdl2_keyboard_poll(void) {
   process_sdl2_events();
}

KEYBOARD_DRIVER keyboard_sdl2 =
{
   .id = KEYBOARD_SDL2,
   .name = empty_string,
   .desc = empty_string,
   .ascii_name = "SDL2 keyboard",
   .autorepeat = FALSE,           // set to TRUE if you want allegro to repeat chars
   .init = sdl2_keyboard_init,
   .exit = sdl2_keyboard_exit,
   .poll = sdl2_keyboard_poll,   // AL_METHOD(void, poll, (void));
   // NULL,   // AL_METHOD(void, set_leds, (int leds));
   // NULL,   // AL_METHOD(void, set_rate, (int delay, int rate));
   // NULL,   // AL_METHOD(void, wait_for_input, (void));
   // NULL,   // AL_METHOD(void, stop_waiting_for_input, (void));
   // NULL,   // AL_METHOD(int,  scancode_to_ascii, (int scancode));
   // NULL    // scancode_to_name
};