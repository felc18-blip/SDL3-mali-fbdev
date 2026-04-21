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
 * SDL_maliopengles.c — Mali FBDEV OpenGL ES portado para SDL3
 *
 * Mudanças em relação ao original SDL2:
 *
 *   MALI_GLES_LoadLibrary
 *     SDL2: int  MALI_GLES_LoadLibrary(_THIS, const char *path)
 *               return SDL_EGL_LoadLibrary(_this, path, EGL_DEFAULT_DISPLAY, 0);
 *               → retornava int (0/-1)
 *     SDL3: bool MALI_GLES_LoadLibrary(SDL_VideoDevice *_this, const char *path)
 *               return SDL_EGL_LoadLibrary(_this, path, EGL_DEFAULT_DISPLAY, 0);
 *               → retorna bool (true/false)
 *               A assinatura de SDL_EGL_LoadLibrary também ganhou os dois
 *               parâmetros extras (native_display, platform) em SDL3 — já
 *               presentes no original que foi enviado, mantidos aqui.
 *
 *   SDL_EGL_CreateContext_impl(MALI)
 *   SDL_EGL_SwapWindow_impl(MALI)
 *   SDL_EGL_MakeCurrent_impl(MALI)
 *     Estas macros continuam disponíveis em SDL3 via SDL_egl_c.h e geram
 *     as funções MALI_GLES_CreateContext, MALI_GLES_SwapWindow e
 *     MALI_GLES_MakeCurrent respectivamente, com as assinaturas SDL3 corretas.
 *
 *   _THIS
 *     A macro _THIS foi removida em SDL3; o parâmetro agora é
 *     SDL_VideoDevice *_this explicitamente. As macros impl acima já lidam
 *     com isso internamente.
 */

#include "SDL_internal.h"

#if defined(SDL_VIDEO_DRIVER_MALI) && defined(SDL_VIDEO_OPENGL_EGL)

#include "SDL_maliopengles.h"
#include "SDL_malivideo.h"

/*
 * Carrega a biblioteca EGL.
 *
 * SDL2: int  — retornava SDL_EGL_LoadLibrary(...) diretamente (int 0/-1)
 * SDL3: bool — SDL_EGL_LoadLibrary retorna bool; repassamos o valor
 *
 * Os 4 parâmetros de SDL_EGL_LoadLibrary em SDL3:
 *   1. SDL_VideoDevice *_this
 *   2. const char      *path           — NULL = lib padrão do sistema
 *   3. NativeDisplayType native_display — EGL_DEFAULT_DISPLAY para fbdev
 *   4. EGLenum           platform       — 0 = EGL escolhe automaticamente
 */
bool MALI_GLES_LoadLibrary(SDL_VideoDevice *_this, const char *path)
{
    return SDL_EGL_LoadLibrary(_this, path, EGL_DEFAULT_DISPLAY, 0);
}

/*
 * As três macros abaixo expandem para implementações completas das funções:
 *
 *   SDL_EGL_CreateContext_impl(MALI)
 *     → SDL_GLContext MALI_GLES_CreateContext(SDL_VideoDevice*, SDL_Window*)
 *
 *   SDL_EGL_SwapWindow_impl(MALI)
 *     → bool MALI_GLES_SwapWindow(SDL_VideoDevice*, SDL_Window*)
 *
 *   SDL_EGL_MakeCurrent_impl(MALI)
 *     → bool MALI_GLES_MakeCurrent(SDL_VideoDevice*, SDL_Window*, SDL_GLContext)
 *
 * Em SDL3 essas macros já estão definidas em SDL_egl_c.h com as assinaturas
 * corretas (bool em vez de int, sem _THIS). Não é necessário nenhuma alteração.
 */
SDL_EGL_CreateContext_impl(MALI)
SDL_EGL_SwapWindow_impl(MALI)
SDL_EGL_MakeCurrent_impl(MALI)

#endif /* SDL_VIDEO_DRIVER_MALI && SDL_VIDEO_OPENGL_EGL */

/* vi: set ts=4 sw=4 expandtab: */
