#include "../../SDL_internal.h"

#if SDL_VIDEO_DRIVER_MALI

#include "../SDL_sysvideo.h"
#include <SDL3/SDL_version.h>
#include <SDL3/SDL_events.h>
#include "../../events/SDL_events_c.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef SDL_INPUT_LINUXEV
#include "../../core/linux/SDL_evdev.h"
#endif

#include "SDL_malivideo.h"
#include "SDL_maliopengles.h"

/* 🔥 FIX FREE (IMPORTANTE) */
static void MALI_DeleteDevice(SDL_VideoDevice *device)
{
    SDL_free(device);
}

static SDL_VideoDevice *MALI_CreateDevice(void);

VideoBootStrap MALI_bootstrap = {
    "mali",
    "Mali EGL Video Driver",
    MALI_CreateDevice,
    NULL
};

/* ================= INIT ================= */
bool MALI_VideoInit(SDL_VideoDevice *_this)
{
    SDL_VideoDisplay display;
    SDL_DisplayMode current_mode;
    int fd;
    struct fb_var_screeninfo vinfo;

    fd = open("/dev/fb0", O_RDWR, 0);

    if (fd < 0) {
        vinfo.xres = 640;
        vinfo.yres = 480;
    } else {
        if (ioctl(fd, FBIOGET_VSCREENINFO, &vinfo) < 0) {
            vinfo.xres = 640;
            vinfo.yres = 480;
        } else {
            vinfo.yres_virtual = vinfo.yres * 2;
            ioctl(fd, FBIOPUT_VSCREENINFO, &vinfo);
        }
        close(fd);
    }

    SDL_zero(current_mode);
    current_mode.w = vinfo.xres;
    current_mode.h = vinfo.yres;
    current_mode.refresh_rate = 60;
    current_mode.format = SDL_PIXELFORMAT_RGBX8888;

    SDL_zero(display);
    display.desktop_mode = current_mode;

    SDL_AddVideoDisplay(&display, false);

    #ifdef SDL_INPUT_LINUXEV
    SDL_EVDEV_Init();
    #endif

    return true;
}

/* ================= QUIT ================= */
void MALI_VideoQuit(SDL_VideoDevice *_this)
{
    #ifdef SDL_INPUT_LINUXEV
    SDL_EVDEV_Quit();
    #endif
}

/* ================= DISPLAY ================= */
bool MALI_GetDisplayModes(SDL_VideoDevice *_this, SDL_VideoDisplay *display)
{
    return true;
}

bool MALI_SetDisplayMode(SDL_VideoDevice *_this, SDL_VideoDisplay * display, SDL_DisplayMode * mode)
{
    return true;
}

/* ================= WINDOW ================= */
bool MALI_CreateWindow(SDL_VideoDevice *_this, SDL_Window *window, SDL_PropertiesID props)
{
    (void)props;

    SDL_WindowData *windowdata;

    windowdata = (SDL_WindowData *) SDL_calloc(1, sizeof(SDL_WindowData));
    if (windowdata == NULL) {
        SDL_OutOfMemory();
        return false;
    }

    /* 🔥 FIX: salva ponteiro */
    SDL_SetPointerProperty(SDL_GetWindowProperties(window), "mali_windowdata", windowdata);

    window->w = 640;
    window->h = 480;

    window->flags |= SDL_WINDOW_OPENGL;

    /* 🔥 FIX: não quebrar no PC */
    if (!_this->egl_data) {
        SDL_GL_LoadLibrary(NULL);
    }

    windowdata->egl_surface = EGL_NO_SURFACE;

    SDL_SetMouseFocus(window);
    SDL_SetKeyboardFocus(window);

    return true;
}

void MALI_DestroyWindow(SDL_VideoDevice *_this, SDL_Window *window)
{
    SDL_PropertiesID props = SDL_GetWindowProperties(window);

    SDL_WindowData *windowdata = SDL_GetPointerProperty(
        props,
        "mali_windowdata",
        NULL
    );

    if (windowdata) {
        SDL_free(windowdata);
        SDL_SetPointerProperty(props, "mali_windowdata", NULL);
    }
}

void MALI_SetWindowTitle(SDL_VideoDevice *_this, SDL_Window * window) {}

bool MALI_SetWindowPosition(SDL_VideoDevice *_this, SDL_Window *window)
{
    return true;
}

void MALI_SetWindowSize(SDL_VideoDevice *_this, SDL_Window * window) {}
void MALI_ShowWindow(SDL_VideoDevice *_this, SDL_Window * window) {}
void MALI_HideWindow(SDL_VideoDevice *_this, SDL_Window * window) {}

/* ================= DEVICE ================= */
static SDL_VideoDevice *MALI_CreateDevice(void)
{
    SDL_VideoDevice *device = SDL_calloc(1, sizeof(SDL_VideoDevice));
    if (!device) {
        SDL_OutOfMemory();
        return NULL;
    }

    device->VideoInit = MALI_VideoInit;
    device->VideoQuit = MALI_VideoQuit;

    device->GetDisplayModes = MALI_GetDisplayModes;
    device->SetDisplayMode = MALI_SetDisplayMode;

    device->CreateSDLWindow = MALI_CreateWindow;
    device->DestroyWindow = MALI_DestroyWindow;

    device->SetWindowSize = MALI_SetWindowSize;
    device->SetWindowPosition = MALI_SetWindowPosition;
    device->SetWindowTitle = MALI_SetWindowTitle;

    device->ShowWindow = MALI_ShowWindow;
    device->HideWindow = MALI_HideWindow;

    device->PumpEvents = MALI_PumpEvents;

    device->GL_LoadLibrary = NULL;
    device->GL_CreateContext = MALI_GLES_CreateContext;
    device->GL_MakeCurrent = MALI_GLES_MakeCurrent;
    device->GL_SwapWindow = MALI_GLES_SwapWindow;

    /* 🔥 FIX CRÍTICO */
    device->free = MALI_DeleteDevice;

    return device;
}

/* ================= EVENTS ================= */
void MALI_PumpEvents(SDL_VideoDevice *_this)
{
    #ifdef SDL_INPUT_LINUXEV
    SDL_EVDEV_Poll();
    #endif
}

#endif
