/*
 S imple DirectMedia Layer        *
 Copyright (C) 1997-2024 Sam Lantinga <slouken@libsdl.org>

 This software is provided 'as-is', without any express or implied
 warranty.  In no event will the authors be held liable for any damages
 arising from the use of this software.

 Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
claim that you wrote the original software. If you use this software
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

/*
 * SDL_malivideo.c — Mali FBDEV driver portado do SDL2 para SDL3
 *
 * ── Resumo das mudanças em relação ao original SDL2 ──────────────────────
 *
 * [Retornos]
 *   SDL2  int  0 / -1           →  SDL3  bool  true / false
 *   SDL_OutOfMemory() retornava int  →  agora é void; chamamos return false
 *   SDL_SetError()    retornava int  →  agora é void; chamamos return false
 *
 * [Display]
 *   display.driverdata            →  removido; dados em SDL_PropertiesID
 *   display.current_mode          →  removido; SDL3 usa só desktop_mode
 *   SDL_AddVideoDisplay()         →  retorna SDL_DisplayID (0 = erro)
 *   SDL_GetDisplayDriverData()    →  removido; usamos SDL_GetDisplayProperties
 *   SDL_AddDisplayMode()          →  removido; GetDisplayModes retorna bool
 *
 * [Window]
 *   window->driverdata            →  removido; dados em SDL_PropertiesID
 *   MALI_CreateWindow             →  assinatura ganhou SDL_PropertiesID
 *
 * [EGL]
 *   SDL_GL_LoadLibrary()          →  SDL_EGL_LoadLibrary() direto (4 args)
 *   SDL_EGL_CreateSurface()       →  assinatura atualizada
 *   SDL_EGL_MakeCurrent()         →  parâmetro SDL_Window* removido (3 args)
 *
 * [SetWindowPosition]
 *   void MALI_SetWindowPosition   →  bool MALI_SetWindowPosition
 *
 * [VideoQuit]
 *   VT_ACTIVATE + setterm         →  removidos (causavam crash em embarcados)
 * ─────────────────────────────────────────────────────────────────────────
 */

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

/* Chaves de propriedade para associar dados internos a displays e janelas */
#define MALI_PROP_DISPLAYDATA   "mali.displaydata"
#define MALI_PROP_WINDOWDATA    "mali.windowdata"

/* =========================================================================
 * Bootstrap / CreateDevice / DeleteDevice
 * ========================================================================= */

static SDL_VideoDevice *MALI_CreateDevice(void);

VideoBootStrap MALI_bootstrap = {
    "mali",
    "Mali EGL Video Driver",
    MALI_CreateDevice,
    NULL
};

static void MALI_DeleteDevice(SDL_VideoDevice *device)
{
    SDL_free(device);
}

static SDL_VideoDevice *MALI_CreateDevice(void)
{
    SDL_VideoDevice *device = (SDL_VideoDevice *)SDL_calloc(1, sizeof(SDL_VideoDevice));
    if (!device) {
        SDL_OutOfMemory();
        return NULL;
    }

    device->VideoInit          = MALI_VideoInit;
    device->VideoQuit          = MALI_VideoQuit;

    device->GetDisplayModes    = MALI_GetDisplayModes;
    device->SetDisplayMode     = MALI_SetDisplayMode;

    device->CreateSDLWindow    = MALI_CreateWindow;
    device->DestroyWindow      = MALI_DestroyWindow;

    device->SetWindowTitle     = MALI_SetWindowTitle;
    device->SetWindowPosition  = MALI_SetWindowPosition;
    device->SetWindowSize      = MALI_SetWindowSize;
    device->ShowWindow         = MALI_ShowWindow;
    device->HideWindow         = MALI_HideWindow;

    device->PumpEvents         = MALI_PumpEvents;

    /* EGL / OpenGL ES — GL_LoadLibrary é NULL porque usamos SDL_EGL direto */
    device->GL_LoadLibrary     = MALI_GLES_LoadLibrary;
    device->GL_GetProcAddress  = SDL_EGL_GetProcAddressInternal;
    device->GL_UnloadLibrary   = SDL_EGL_UnloadLibrary;
    device->GL_CreateContext   = MALI_GLES_CreateContext;
    device->GL_MakeCurrent     = MALI_GLES_MakeCurrent;
    device->GL_SwapWindow      = MALI_GLES_SwapWindow;
    device->GL_DestroyContext  = MALI_GLES_DeleteContext;

    device->free               = MALI_DeleteDevice;

    return device;
}

/* =========================================================================
 * VideoInit
 *
 * SDL2: retornava int (0 / -1)
 * SDL3: retorna bool (true / false)
 * ========================================================================= */

bool MALI_VideoInit(SDL_VideoDevice *_this)
{
    SDL_VideoDisplay          display;
    SDL_DisplayMode           current_mode;
    SDL_DisplayData          *data;
    SDL_DisplayID             display_id;
    int                       fd;
    struct fb_var_screeninfo  vinfo;

    /* Aloca dados internos do display */
    data = (SDL_DisplayData *)SDL_calloc(1, sizeof(SDL_DisplayData));
    if (!data) {
        SDL_OutOfMemory();
        return false;
    }

    /* Lê resolução real do framebuffer */
    fd = open("/dev/fb0", O_RDWR, 0);
    if (fd < 0) {
        SDL_LogWarn(SDL_LOG_CATEGORY_VIDEO,
                    "mali-fbdev: Could not open /dev/fb0, using 640x480");
        vinfo.xres = 640;
        vinfo.yres = 480;
    } else {
        if (ioctl(fd, FBIOGET_VSCREENINFO, &vinfo) < 0) {
            SDL_LogWarn(SDL_LOG_CATEGORY_VIDEO,
                        "mali-fbdev: FBIOGET_VSCREENINFO failed, using 640x480");
            vinfo.xres = 640;
            vinfo.yres = 480;
        } else {
            /* Habilita double-buffering no fb */
            vinfo.yres_virtual = vinfo.yres * 2;
            if (ioctl(fd, FBIOPUT_VSCREENINFO, &vinfo) < 0) {
                SDL_LogWarn(SDL_LOG_CATEGORY_VIDEO,
                            "mali-fbdev: Could not set virtual screen size (non-fatal)");
            }
        }
        close(fd);
    }

    data->native_display.width  = (int)vinfo.xres;
    data->native_display.height = (int)vinfo.yres;

    /* Modo de vídeo */
    SDL_zero(current_mode);
    current_mode.w            = (int)vinfo.xres;
    current_mode.h            = (int)vinfo.yres;
    current_mode.refresh_rate = 60.0f;
    current_mode.format       = SDL_PIXELFORMAT_RGBX8888;

    /*
     * SDL2 tinha display.current_mode — SDL3 removeu; só desktop_mode existe.
     * SDL2 tinha display.driverdata   — SDL3 removeu; usamos propriedades.
     */
    SDL_zero(display);
    display.desktop_mode = current_mode;

    /* SDL3: SDL_AddVideoDisplay retorna SDL_DisplayID; 0 indica erro */
    display_id = SDL_AddVideoDisplay(&display, false);
    if (display_id == 0) {
        SDL_free(data);
        return false;
    }

    /* Associa os dados do driver ao display via sistema de propriedades SDL3 */
    SDL_SetPointerProperty(
        SDL_GetDisplayProperties(display_id),
                           MALI_PROP_DISPLAYDATA,
                           data
    );

    #ifdef SDL_INPUT_LINUXEV
    if (!SDL_EVDEV_Init()) {
        SDL_LogWarn(SDL_LOG_CATEGORY_INPUT,
                    "mali-fbdev: SDL_EVDEV_Init failed: %s (non-fatal)", SDL_GetError());
    }
    #endif

    return true;
}

/* =========================================================================
 * VideoQuit
 *
 * SDL2 original fazia VT_ACTIVATE + setterm — removidos porque:
 *   - Causam crash em sistemas embarcados sem VT
 *   - O SDL3 não faz isso em nenhum outro driver fbdev/kmsdrm
 *   - A aplicação pode fazer isso manualmente se necessário
 * ========================================================================= */

void MALI_VideoQuit(SDL_VideoDevice *_this)
{
    #ifdef SDL_INPUT_LINUXEV
    SDL_EVDEV_Quit();
    #endif
}

/* =========================================================================
 * Display
 * ========================================================================= */

/*
 * SDL2: void — chamava SDL_AddDisplayMode()
 * SDL3: bool — registra explicitamente um fullscreen mode correspondente ao
 *              framebuffer nativo. Sem isso, apps que chamam
 *              SDL_GetFullscreenDisplayModes() recebem lista vazia e falham
 *              com "Cannot find desired video mode" (bug visto em re3/reVC).
 */
bool MALI_GetDisplayModes(SDL_VideoDevice *_this, SDL_VideoDisplay *display)
{
    SDL_DisplayData *data;
    SDL_DisplayMode  mode;

    (void)_this;

    data = (SDL_DisplayData *)SDL_GetPointerProperty(
        SDL_GetDisplayProperties(display->id),
        MALI_PROP_DISPLAYDATA,
        NULL
    );
    if (!data) {
        return true;  /* sem dados ainda; SDL usa desktop_mode */
    }

    SDL_zero(mode);
    mode.w            = data->native_display.width;
    mode.h            = data->native_display.height;
    mode.refresh_rate = 60.0f;
    mode.format       = SDL_PIXELFORMAT_RGBX8888;

    if (!SDL_AddFullscreenDisplayMode(display, &mode)) {
        return false;
    }
    return true;
}

/*
 * SDL2: int (0 / -1)
 * SDL3: bool (true / false)
 */
bool MALI_SetDisplayMode(SDL_VideoDevice *_this,
                         SDL_VideoDisplay *display,
                         SDL_DisplayMode  *mode)
{
    (void)_this; (void)display; (void)mode;
    return true;   /* fbdev não suporta troca de modo em runtime */
}

/* =========================================================================
 * Helpers internos
 * ========================================================================= */

/*
 * Recupera SDL_DisplayData do primeiro display registrado.
 *
 * SDL2 usava SDL_GetDisplayDriverData(0) — removido em SDL3.
 * SDL3: iteramos os displays e lemos a propriedade que gravamos no Init.
 */
static SDL_DisplayData *MALI_GetDisplayData(void)
{
    SDL_DisplayID   *displays;
    int              num_displays = 0;
    SDL_DisplayData *data = NULL;

    displays = SDL_GetDisplays(&num_displays);
    if (displays && num_displays > 0) {
        data = (SDL_DisplayData *)SDL_GetPointerProperty(
            SDL_GetDisplayProperties(displays[0]),
                                                         MALI_PROP_DISPLAYDATA,
                                                         NULL
        );
    }
    SDL_free(displays);
    return data;
}

/* =========================================================================
 * Window
 * ========================================================================= */

/*
 * SDL2: int MALI_CreateWindow(SDL_VideoDevice*, SDL_Window*)
 * SDL3: bool MALI_CreateWindow(SDL_VideoDevice*, SDL_Window*, SDL_PropertiesID)
 *
 * SDL2 usava window->driverdata — SDL3 removeu; usamos SDL_PropertiesID.
 */
bool MALI_CreateWindow(SDL_VideoDevice *_this,
                       SDL_Window      *window,
                       SDL_PropertiesID create_props)
{
    SDL_WindowData  *windowdata;
    SDL_DisplayData *displaydata;

    (void)create_props;   /* não usamos propriedades de criação por ora */

    displaydata = MALI_GetDisplayData();
    if (!displaydata) {
        SDL_SetError("mali-fbdev: No display data available");
        return false;
    }

    windowdata = (SDL_WindowData *)SDL_calloc(1, sizeof(SDL_WindowData));
    if (!windowdata) {
        SDL_OutOfMemory();
        return false;
    }

    /* SDL3: window->internal é o novo driverdata; a macro SDL_EGL_CreateContext_impl 
     * acessa window->internal->egl_surface, então precisa ser setado. */
    window->internal = windowdata;

    /* Janela ocupa a tela inteira — fbdev não tem gerenciador de janelas.
     * Sinalizar fullscreen+borderless deixa o app ciente de que não há
     * espaço pra decoração/resize e que a janela é o display inteiro. */
    window->w = displaydata->native_display.width;
    window->h = displaydata->native_display.height;
    window->flags |= SDL_WINDOW_FULLSCREEN | SDL_WINDOW_BORDERLESS;


    /*
     * Carrega EGL se necessário.
     *
     * SDL2: SDL_GL_LoadLibrary(NULL)  →  retornava int
     * SDL3: SDL_EGL_LoadLibrary(_this, path, native_display, platform)
     *         path           = NULL            → lib padrão do sistema
     *         native_display = EGL_DEFAULT_DISPLAY
     *         platform       = 0               → EGL escolhe automaticamente
     */
    /* Só criar EGL surface se a janela for OpenGL.
     * Se a janela vier sem SDL_WINDOW_OPENGL, o SDL3 vai chamar RecreateWindow
     * com a flag se o usuário criar renderer opengles2 — aí sim criamos o EGL.
     */
    if (window->flags & SDL_WINDOW_OPENGL) {
        if (!_this->egl_data) {
            if (!SDL_EGL_LoadLibrary(_this, NULL, EGL_DEFAULT_DISPLAY, 0)) {
                SDL_free(windowdata);
                return false;
            }
        }
        _this->egl_data->egl_surfacetype = EGL_WINDOW_BIT;

        windowdata->egl_surface = SDL_EGL_CreateSurface(
            _this,
            window,
            (NativeWindowType)&displaydata->native_display
        );

        if (windowdata->egl_surface == EGL_NO_SURFACE) {
            SDL_SetError("mali-fbdev: Can't create EGL window surface");
            SDL_free(windowdata);
            return false;
        }
    } else {
        windowdata->egl_surface = EGL_NO_SURFACE;
    }

    /*
     * SDL2: window->driverdata = windowdata
     * SDL3: usamos o sistema de propriedades
     */
    SDL_SetPointerProperty(
        SDL_GetWindowProperties(window),
                           MALI_PROP_WINDOWDATA,
                           windowdata
    );

    /* Janela única: sempre com foco */
    SDL_SetMouseFocus(window);
    SDL_SetKeyboardFocus(window);

    return true;
}

void MALI_DestroyWindow(SDL_VideoDevice *_this, SDL_Window *window)
{
    SDL_PropertiesID  props      = SDL_GetWindowProperties(window);
    SDL_WindowData   *windowdata = (SDL_WindowData *)SDL_GetPointerProperty(
        props, MALI_PROP_WINDOWDATA, NULL);

    if (windowdata) {
        if (windowdata->egl_surface != EGL_NO_SURFACE) {
            SDL_EGL_DestroySurface(_this, windowdata->egl_surface);
            windowdata->egl_surface = EGL_NO_SURFACE;
        }
        SDL_free(windowdata);
        SDL_SetPointerProperty(props, MALI_PROP_WINDOWDATA, NULL);
    }
}

/* =========================================================================
 * Window stubs
 *
 * fbdev não tem gerenciador de janelas; estas funções satisfazem a interface
 * SDL_VideoDevice mas não fazem nada de concreto.
 *
 * Mudança importante:
 *   SDL2: void MALI_SetWindowPosition(...)
 *   SDL3: bool MALI_SetWindowPosition(...)   ← retorno mudou
 * ========================================================================= */

void MALI_SetWindowTitle(SDL_VideoDevice *_this, SDL_Window *window)
{
    (void)_this; (void)window;
}

bool MALI_SetWindowPosition(SDL_VideoDevice *_this, SDL_Window *window)
{
    (void)_this; (void)window;
    return true;   /* posição fixa em fbdev; sucesso silencioso */
}

void MALI_SetWindowSize(SDL_VideoDevice *_this, SDL_Window *window)
{
    (void)_this; (void)window;
}

void MALI_ShowWindow(SDL_VideoDevice *_this, SDL_Window *window)
{
    (void)_this; (void)window;
}

void MALI_HideWindow(SDL_VideoDevice *_this, SDL_Window *window)
{
    (void)_this; (void)window;
}

/* =========================================================================
 * Events
 * ========================================================================= */

void MALI_PumpEvents(SDL_VideoDevice *_this)
{
    (void)_this;
    #ifdef SDL_INPUT_LINUXEV
    SDL_EVDEV_Poll();
    #endif
}

#endif /* SDL_VIDEO_DRIVER_MALI */

/* vi: set ts=4 sw=4 expandtab: */
