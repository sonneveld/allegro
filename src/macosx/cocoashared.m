#include "allegro.h"
#include "allegro/internal/aintern.h"
#include "allegro/platform/aintosx.h"

#ifndef ALLEGRO_MACOSX
#error something is wrong with the makefile
#endif

// Quickdraw specific
#ifdef ENABLE_QUICKDRAW
void osx_qz_mark_dirty();
void prepare_window_for_animation(int refresh_view);
#endif

void* osx_window_mutex;

@implementation AllegroWindow

#ifdef ENABLE_QUICKDRAW
/* display:
 *  Called when the window is about to be deminiaturized.
 */
- (void)display
{
    [super display];
    prepare_window_for_animation(TRUE);
}



/* miniaturize:
 *  Called when the window is miniaturized.
 */
- (void)miniaturize: (id)sender
{
    prepare_window_for_animation(FALSE);
    [super miniaturize: sender];
}
#endif

- (BOOL)canBecomeKeyWindow
{
    return YES;
}

- (BOOL)canBecomeMainWindow
{
    return YES;
}

@end


@implementation AllegroWindowDelegate

/* windowShouldClose:
 *  Called when the user attempts to close the window.
 *  Default behaviour is to call the user callback (if any) and deny closing.
 */
- (BOOL)windowShouldClose: (id)sender
{
    if (osx_window_close_hook)
        osx_window_close_hook();
    return NO;
}



/* windowDidDeminiaturize:
 *  Called when the window deminiaturization animation ends; marks the whole
 *  window contents as dirty, so it is updated on next refresh.
 */
- (void)windowDidDeminiaturize: (NSNotification *)aNotification
{
    _unix_lock_mutex(osx_window_mutex);
#ifdef ENABLE_QUICKDRAW
    osx_qz_mark_dirty();
#endif
    _unix_unlock_mutex(osx_window_mutex);
}



/* windowDidBecomeKey:
 * Sent by the default notification center immediately after an NSWindow
 * object has become key.
 */
- (void)windowDidBecomeKey:(NSNotification *)notification
{
    _unix_lock_mutex(osx_skip_events_processing_mutex);
    osx_skip_events_processing = FALSE;
    _unix_unlock_mutex(osx_skip_events_processing_mutex);
}



/* windowDidResignKey:
 * Sent by the default notification center immediately after an NSWindow
 * object has resigned its status as key window.
 */
- (void)windowDidResignKey:(NSNotification *)notification
{
    _unix_lock_mutex(osx_skip_events_processing_mutex);
    osx_skip_events_processing = TRUE;
    _unix_unlock_mutex(osx_skip_events_processing_mutex);
}

@end

/* Local variables:       */
/* c-basic-offset: 3      */
/* indent-tabs-mode: nil  */
/* c-file-style: "linux" */
/* End:                   */
