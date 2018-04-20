#include "allegro.h"
#include "allegro/internal/aintern.h"

#include "SDL.h"
#include <assert.h>

static int sdl2_sys_init(void)
{
  
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_TIMER) != 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return 1;
    }
   
   /* Setup OS type & version */
   os_type = OSTYPE_SDL2;
   os_version = 0; // (((result >> 12) & 0xf) * 10) + ((result >> 8) & 0xf);
   os_revision = 0; // (result >> 4) & 0xf;
   os_multitasking = TRUE;
   
   return 0;
}



static void sdl2_sys_exit(void)
{
   SDL_Quit();
}

int _unix_find_resource(char *dest, AL_CONST char *resource, int size);

static void sdl2_sys_set_window_title(AL_CONST char *title) {

}

static int sdl2_sys_set_close_button_callback(void (*proc)(void)) {
   return 0;
}

static void sdl2_sys_message(AL_CONST char *msg) {

}

static int sdl2_sys_set_display_switch_mode(int mode) {
   return -1;
}
 
static int sdl2_sys_desktop_color_depth(void) {
   return -1;
}
static int sdl2_sys_get_desktop_resolution(int *width, int *height) {
   return -1;
}

static void sdl2_sys_get_gfx_safe_mode(int *driver, struct GFX_MODE *mode)
{
   *driver = GFX_SDL2;
   mode->width = 320;
   mode->height = 200;
   mode->bpp = 32;
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
   SYSTEM_SDL2,
   empty_string,
   empty_string,
   "MacOS X",
   sdl2_sys_init,
   sdl2_sys_exit,
   NULL, // osx_sys_get_executable_name,
   _unix_find_resource,
   sdl2_sys_set_window_title,
   sdl2_sys_set_close_button_callback,
   sdl2_sys_message,
   NULL,  /* AL_METHOD(void, assert, (AL_CONST char *msg)); */
   NULL,  /* AL_METHOD(void, save_console_state, (void)); */
   NULL,  /* AL_METHOD(void, restore_console_state, (void)); */
   NULL,  /* AL_METHOD(struct BITMAP *, create_bitmap, (int color_depth, int width, int height)); */
   NULL,  /* AL_METHOD(void, created_bitmap, (struct BITMAP *bmp)); */
   NULL,  /* AL_METHOD(struct BITMAP *, create_sub_bitmap, (struct BITMAP *parent, int x, int y, int width, int height)); */
   NULL,  /* AL_METHOD(void, created_sub_bitmap, (struct BITMAP *bmp, struct BITMAP *parent)); */
   NULL,  /* AL_METHOD(int, destroy_bitmap, (struct BITMAP *bitmap)); */
   NULL,  /* AL_METHOD(void, read_hardware_palette, (void)); */
   NULL,  /* AL_METHOD(void, set_palette_range, (AL_CONST struct RGB *p, int from, int to, int retracesync)); */
   NULL,  /* AL_METHOD(struct GFX_VTABLE *, get_vtable, (int color_depth)); */
   sdl2_sys_set_display_switch_mode,
   NULL,  /* AL_METHOD(void, display_switch_lock, (int lock, int foreground)); */
   sdl2_sys_desktop_color_depth,
   sdl2_sys_get_desktop_resolution,
   sdl2_sys_get_gfx_safe_mode,
   NULL, //_unix_yield_timeslice,
   sdl2_create_mutex,
   sdl2_destroy_mutex,
   sdl2_lock_mutex,
   sdl2_unlock_mutex,
   NULL,  /* AL_METHOD(_DRIVER_INFO *, gfx_drivers, (void)); */
   NULL,  /* AL_METHOD(_DRIVER_INFO *, digi_drivers, (void)); */
   NULL,  /* AL_METHOD(_DRIVER_INFO *, midi_drivers, (void)); */
   NULL,  /* AL_METHOD(_DRIVER_INFO *, keyboard_drivers, (void)); */
   NULL,  /* AL_METHOD(_DRIVER_INFO *, mouse_drivers, (void)); */
   NULL,  /* AL_METHOD(_DRIVER_INFO *, joystick_drivers, (void)); */
   NULL,  /* AL_METHOD(_DRIVER_INFO *, timer_drivers, (void)); */
};


#define TIMER_TO_USEC(x)  ((long)((x) / 1193.181))
// #define MSEC_TO_TIMER(x)  ((long)((x) * (TIMERS_PER_SECOND / 1000.)))

static SDL_TimerID my_timer_id;

Uint32 sdl2_timerint(Uint32 interval_ms, void *param)
{
   int interval_timer =  _handle_timer_tick(MSEC_TO_TIMER(interval_ms));

   return TIMER_TO_USEC(interval_timer);
}



static int sdl2_timer_init(void)
{
   my_timer_id = SDL_AddTimer(27, sdl2_timerint, NULL);
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


static int digi_sdl2_detect(int input)
{
   if (input)
      return 0;

   return 1;
}


static   SDL_AudioDeviceID dev;

void MyAudioCallback(void*  userdata,
                       Uint8* stream,
                       int    len) 
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
   want.samples = 1024;  // number of frames
   want.callback = MyAudioCallback; 

   dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);

   if (_mixer_init(have.samples * have.channels, have.freq,
                    TRUE, 1, &digi_sdl2.voices) != 0) {
      // _TRACE(PREFIX_E "Can't init software mixer\n");
      // goto Error;
      return -1;
   }

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


static BITMAP *gfx_sdl2_init(int w, int h, int v_w, int v_h, int color_depth) {


    if (color_depth != 32)
        ustrzcpy(allegro_error, ALLEGRO_ERROR_SIZE, get_config_text("Unsupported color depth"));

    displayed_video_bitmap = create_bitmap_ex(color_depth, w, h);
    

    return displayed_video_bitmap;

}
static void gfx_sdl2_exit(BITMAP *bmp) {

}


GFX_DRIVER gfx_sdl2 =
{
   GFX_SDL2,
   empty_string,
   empty_string,
   "SDL2 Windowed",
   gfx_sdl2_init,
   gfx_sdl2_exit,
   NULL,
   NULL,               //_xwin_vsync,
   NULL,
   NULL, NULL, NULL,
   NULL, //gfx_sdl2_create_video_bitmap,
   NULL, // gfx_sdl2_destroy_video_bitmap,
   NULL, NULL,					/* No show/request video bitmaps */
   NULL, NULL,
   NULL, // gfx_sdl2_set_mouse_sprite,
   NULL, // gfx_sdl2_show_mouse,
   NULL, // gfx_sdl2_hide_mouse,
   NULL, // gfx_sdl2_move_mouse,
   NULL,
   NULL, NULL,
   NULL,                        /* No fetch_mode_list */
   0, 0,
   0,
   0, 0,
   0,
   0,
   TRUE                         /* Windowed mode */
};