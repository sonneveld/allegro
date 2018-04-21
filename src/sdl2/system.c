#include "allegro.h"
#include "allegro/internal/aintern.h"

#include "SDL.h"
#include <assert.h>

const int DEFAULT_DISPLAY_INDEX = 0;


// ----------------------------------------------------------------------------
// EVENTS!
// ----------------------------------------------------------------------------

void process_sdl2_events (void) {
   
   SDL_PumpEvents();         

   SDL_Event event;
   while (SDL_PollEvent(&event)) {
      // printf(" - %d\n", event.type);
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
   SDL_Log("TODO when events are processed");
   return -1;
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
   return SDL_CreateMutex();
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
   want.samples = 1024;  // number of frames, not samples
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

static BITMAP *gfx_sdl2_init(int w, int h, int v_w, int v_h, int color_depth) {

   if (color_depth != 32) {
      ustrzcpy(allegro_error, ALLEGRO_ERROR_SIZE, get_config_text("Unsupported color depth"));
      return NULL;
   }

   displayed_video_bitmap = create_bitmap_ex(color_depth, w, h);

   window = SDL_CreateWindow(
        window_title,                  // window title
        SDL_WINDOWPOS_CENTERED_DISPLAY(DEFAULT_DISPLAY_INDEX),           // initial x position
        SDL_WINDOWPOS_CENTERED_DISPLAY(DEFAULT_DISPLAY_INDEX),           // initial y position
        w,                               // width, in pixels
        h,                               // height, in pixels
        SDL_WINDOW_ALLOW_HIGHDPI   //|SDL_WINDOW_FULLSCREEN_DESKTOP                 // flags - see below
    );

   renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
   screenTex = SDL_CreateTexture( renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, w, h );

   gfx_sdl2.w = w;
   gfx_sdl2.h = h;

   return displayed_video_bitmap;
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

   unsigned char * pixels;
   int pitch;

   SDL_LockTexture(screenTex, NULL, (void **)&pixels, &pitch);

   for (int i = 0; i < displayed_video_bitmap->h; i++) {
      memcpy(pixels, displayed_video_bitmap->line[i], pitch);
      pixels += pitch;
   }

   SDL_UnlockTexture(screenTex);

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

GFX_DRIVER gfx_sdl2 =
{
   .id = GFX_SDL2,
   .name = empty_string,
   .desc = empty_string,
   .ascii_name = "SDL2 Windowed",
   .init = gfx_sdl2_init,
   .exit = gfx_sdl2_exit,
   //.vsync = gfx_sdl2_vsync,             
   // .create_video_bitmap = NULL, //gfx_sdl2_create_video_bitmap,
   // .destroy_video_bitmap = NULL, // gfx_sdl2_destroy_video_bitmap,
   // .set_mouse_sprite = NULL, // gfx_sdl2_set_mouse_sprite,
   .show_mouse = gfx_sdl2_show_mouse, // gfx_sdl2_show_mouse,
   .hide_mouse = gfx_sdl2_hide_mouse, // gfx_sdl2_hide_mouse,
   // .move_mouse = NULL, // gfx_sdl2_move_mouse,
   .linear = TRUE,
   .windowed = TRUE                   
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
   int x;
   int y;

   int buttons = SDL_GetMouseState(&x, &y);

   int new_mouse_b = 0;
   if (buttons & SDL_BUTTON (SDL_BUTTON_LEFT)) {
      new_mouse_b |= 1;
   }
   if (buttons & SDL_BUTTON (SDL_BUTTON_RIGHT)) {
      new_mouse_b |= 2;
   }
   if (buttons & SDL_BUTTON (SDL_BUTTON_MIDDLE)) {
      new_mouse_b |= 4;
   }
   if (buttons & SDL_BUTTON (SDL_BUTTON_X1)) {
      new_mouse_b |= 8;
   }
   if (buttons & SDL_BUTTON (SDL_BUTTON_X2)) {
      new_mouse_b |= 16;
   }

   _mouse_b = new_mouse_b;
   _mouse_x = x;
   _mouse_y = y;

   _handle_mouse_input();
}

static void sdl2_mouse_position(int x, int y) {
   SDL_WarpMouseInWindow(window, x, y);
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
   .position = sdl2_mouse_position,
   // .set_range = sdl2_mouse_set_range,
   // .set_speed = NULL,       // AL_METHOD(void, set_speed, (int xspeed, int yspeed));
   // .get_mickeys = sdl2_mouse_get_mickeys,
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

KEYBOARD_DRIVER keyboard_sdl2 =
{
   .id = KEYBOARD_SDL2,
   .name = empty_string,
   .desc = empty_string,
   .ascii_name = "SDL2 keyboard",
   .autorepeat = TRUE,
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