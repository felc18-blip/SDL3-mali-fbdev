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
 * SDL_maliopengles.h — Mali FBDEV OpenGL ES header portado para SDL3
 *
 * Mudanças em relação ao original SDL2:
 *   - _THIS substituído por SDL_VideoDevice *_this  (macro removida em SDL3)
 *   - MALI_GLES_LoadLibrary: retorno int → bool
 *   - MALI_GLES_SwapWindow:  retorno int → bool
 *   - MALI_GLES_MakeCurrent: retorno int → bool
 *   - SDL_EGL_CreateContext_impl / SDL_EGL_SwapWindow_impl /
 *     SDL_EGL_MakeCurrent_impl  continuam disponíveis em SDL3 via SDL_egl_c.h
 */

#ifndef SDL_maliopengles_h
#define SDL_maliopengles_h

#if defined(SDL_VIDEO_DRIVER_MALI) && defined(SDL_VIDEO_OPENGL_EGL)

#include "../SDL_sysvideo.h"
#include "../SDL_egl_c.h"

/*
 * Macros de conveniência que reusam a implementação genérica EGL do SDL3.
 * Estes nomes não mudaram entre SDL2 e SDL3.
 */
#define MALI_GLES_GetAttribute    SDL_EGL_GetAttribute
#define MALI_GLES_GetProcAddress  SDL_EGL_GetProcAddress
#define MALI_GLES_UnloadLibrary   SDL_EGL_UnloadLibrary
#define MALI_GLES_SetSwapInterval SDL_EGL_SetSwapInterval
#define MALI_GLES_GetSwapInterval SDL_EGL_GetSwapInterval
/*
 * SDL2: SDL_EGL_DeleteContext
 * SDL3: SDL_EGL_DestroyContext  ← renomeado para consistência com o resto da API
 */
#define MALI_GLES_DeleteContext   SDL_EGL_DestroyContext

/*
 * SDL2: int  MALI_GLES_LoadLibrary(_THIS, const char *path)
 * SDL3: bool MALI_GLES_LoadLibrary(SDL_VideoDevice *_this, const char *path)
 */
extern bool MALI_GLES_LoadLibrary(SDL_VideoDevice *_this, const char *path);

/*
 * SDL3: SDL_EGL_CreateContext_impl(MALI) gera MALI_GLES_CreateContext
 *       SDL_EGL_SwapWindow_impl(MALI)    gera MALI_GLES_SwapWindow   (bool)
 *       SDL_EGL_MakeCurrent_impl(MALI)   gera MALI_GLES_MakeCurrent  (bool)
 *
 * As declarações abaixo espelham o que as macros produzem internamente.
 */
extern SDL_GLContext MALI_GLES_CreateContext(SDL_VideoDevice *_this,
                                             SDL_Window      *window);

extern bool MALI_GLES_SwapWindow(SDL_VideoDevice *_this,
                                 SDL_Window      *window);

extern bool MALI_GLES_MakeCurrent(SDL_VideoDevice *_this,
                                  SDL_Window      *window,
                                  SDL_GLContext    context);

#endif /* SDL_VIDEO_DRIVER_MALI && SDL_VIDEO_OPENGL_EGL */

#endif /* SDL_maliopengles_h */

/* vi: set ts=4 sw=4 expandtab: */
