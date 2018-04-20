
#ifndef ALSDL2_H
#define ALSDL2_H

#ifndef ALLEGRO_SDL2
   #error bad include
#endif


#ifndef SCAN_DEPEND
   #include <stdio.h>
   #include <stdlib.h>
   #include <fcntl.h>
   #include <unistd.h>
   #include <signal.h>
   #include <pthread.h>
   #if defined __OBJC__ && defined ALLEGRO_SRC
      #undef TRUE
      #undef FALSE
      #import <mach/mach.h>
      #import <mach/mach_error.h>
      #import <AppKit/AppKit.h>
      #import <ApplicationServices/ApplicationServices.h>
      #import <Cocoa/Cocoa.h>
      #import <CoreAudio/CoreAudio.h>
      #import <AudioUnit/AudioUnit.h>
      #import <AudioToolbox/AudioToolbox.h>
      #import <QuickTime/QuickTime.h>
      #import <IOKit/IOKitLib.h>
      #import <IOKit/IOCFPlugIn.h>
      #import <IOKit/hid/IOHIDLib.h>
      #import <IOKit/hid/IOHIDKeys.h>
      #import <Kernel/IOKit/hidsystem/IOHIDUsageTables.h>
      #undef TRUE
      #undef FALSE
      #undef assert
      #define TRUE  -1
      #define FALSE 0
   #endif
#endif



/* System driver */
#define SYSTEM_SDL2            AL_ID('S','D','L',' ')
AL_VAR(SYSTEM_DRIVER, system_sdl2);

/* Timer driver */
#define TIMER_SDL2               AL_ID('S','D','L','T')
AL_VAR(TIMER_DRIVER, timer_sdl2);

/* Keyboard driver */
#define KEYBOARD_SDL2              AL_ID('S','D','L','K')
AL_VAR(KEYBOARD_DRIVER, keyboard_sdl2);

/* Mouse drivers */
#define MOUSE_SDL2               AL_ID('S','D','L','M')
AL_VAR(MOUSE_DRIVER, mouse_sdl2);

/* Gfx driver */
#define GFX_SDL2                 AL_ID('S','D','L','G')
AL_VAR(GFX_DRIVER, gfx_sdl2);

/* Digital sound driver */
#define DIGI_SDL2                AL_ID('S','D','L','S')
AL_VAR(DIGI_DRIVER, digi_sdl2);

/* Joystick drivers */
#define JOYSTICK_SDL2            AL_ID('S','D','L','J')
AL_VAR(JOYSTICK_DRIVER, joystick_sdl2);

#endif





