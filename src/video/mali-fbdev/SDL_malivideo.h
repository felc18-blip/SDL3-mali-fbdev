/*
 S imple DirectMedia Layer         *
 */

#ifndef _SDL_malivideo_h
#define _SDL_malivideo_h

#include "../../SDL_internal.h"
#include "../SDL_sysvideo.h"

/* SDL3: EGL interno */
#include "../SDL_egl_c.h"

#include <EGL/egl.h>
#include <linux/vt.h>
#include <linux/fb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

/* Shadow fbdev window */
struct shadow_fbdev_window {
    unsigned short width;
    unsigned short height;
};

typedef struct SDL_DisplayData
{
    struct shadow_fbdev_window native_display;
} SDL_DisplayData;

typedef struct SDL_WindowData
{
    EGLSurface egl_surface;
} SDL_WindowData;

/****************************************************************************/
/* SDL_VideoDevice functions declaration (SDL3 style)                       */
/****************************************************************************/

/* Core */
bool MALI_VideoInit(SDL_VideoDevice *_this);
void MALI_VideoQuit(SDL_VideoDevice *_this);

/* Display */
bool MALI_GetDisplayModes(SDL_VideoDevice *_this, SDL_VideoDisplay *display);
bool MALI_SetDisplayMode(SDL_VideoDevice *_this, SDL_VideoDisplay *display, SDL_DisplayMode *mode);

/* Window */
bool MALI_CreateWindow(SDL_VideoDevice *_this, SDL_Window *window, SDL_PropertiesID props);
void MALI_DestroyWindow(SDL_VideoDevice *_this, SDL_Window *window);

void MALI_SetWindowTitle(SDL_VideoDevice *_this, SDL_Window *window);

/* 🔥 CORRIGIDO (SDL3 exige bool) */
bool MALI_SetWindowPosition(SDL_VideoDevice *_this, SDL_Window *window);

void MALI_SetWindowSize(SDL_VideoDevice *_this, SDL_Window *window);
void MALI_ShowWindow(SDL_VideoDevice *_this, SDL_Window *window);
void MALI_HideWindow(SDL_VideoDevice *_this, SDL_Window *window);

/* Events */
void MALI_PumpEvents(SDL_VideoDevice *_this);

#endif /* _SDL_malivideo_h */

/* vi: set ts=4 sw=4 expandtab: */
