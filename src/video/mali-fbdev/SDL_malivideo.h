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
 * SDL_malivideo.h — Mali FBDEV driver header portado para SDL3
 *
 * Mudanças em relação ao original SDL2:
 *   - MALI_SetWindowPosition: retorno void → bool
 *   - MALI_CreateWindow: assinatura ganhou SDL_PropertiesID
 *   - SDL_DisplayData / SDL_WindowData: campos driverdata removidos;
 *     dados ficam em SDL_PropertiesID via SDL_SetPointerProperty
 *   - Removido SDL_DisplayData *data do topo (era campo de display.driverdata)
 */

#ifndef SDL_malivideo_h
#define SDL_malivideo_h

#include "../SDL_sysvideo.h"
#include "../SDL_egl_c.h"

/* Tipo nativo de janela Mali FBDEV */
typedef struct
{
    unsigned short width;
    unsigned short height;
} Mali_NativeWindowType;

/* Dados internos associados a cada SDL_VideoDisplay */
typedef struct SDL_DisplayData
{
    Mali_NativeWindowType native_display;
} SDL_DisplayData;

/* Dados internos associados a cada SDL_Window */
typedef struct SDL_WindowData
{
    EGLSurface egl_surface;
} SDL_WindowData;

/*-------------------------------------------------------------------------
 * Funções exportadas — declarações com assinaturas SDL3
 *------------------------------------------------------------------------*/

/* Init / Quit */
extern bool MALI_VideoInit(SDL_VideoDevice *_this);
extern void MALI_VideoQuit(SDL_VideoDevice *_this);

/* Display */
extern bool MALI_GetDisplayModes(SDL_VideoDevice *_this, SDL_VideoDisplay *display);
extern bool MALI_SetDisplayMode(SDL_VideoDevice *_this, SDL_VideoDisplay *display, SDL_DisplayMode *mode);

/* Window */
extern bool MALI_CreateWindow(SDL_VideoDevice *_this, SDL_Window *window, SDL_PropertiesID create_props);
extern void MALI_DestroyWindow(SDL_VideoDevice *_this, SDL_Window *window);

/* Window helpers — SDL3: SetWindowPosition retorna bool */
extern void MALI_SetWindowTitle(SDL_VideoDevice *_this, SDL_Window *window);
extern bool MALI_SetWindowPosition(SDL_VideoDevice *_this, SDL_Window *window);
extern void MALI_SetWindowSize(SDL_VideoDevice *_this, SDL_Window *window);
extern void MALI_ShowWindow(SDL_VideoDevice *_this, SDL_Window *window);
extern void MALI_HideWindow(SDL_VideoDevice *_this, SDL_Window *window);

/* Events */
extern void MALI_PumpEvents(SDL_VideoDevice *_this);

#endif /* SDL_malivideo_h */

/* vi: set ts=4 sw=4 expandtab: */
