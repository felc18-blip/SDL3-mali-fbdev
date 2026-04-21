/*
 S imple DirectMedia Layer          *
 Copyright (C) 1997-2014 Sam Lantinga

 This software is provided 'as-is', without any express or implied warranty.
 */
#include "../SDL_sysvideo.h"
#include "../../SDL_internal.h"

#if SDL_VIDEO_DRIVER_MALI && SDL_VIDEO_OPENGL_EGL

/* SDL internals */
#include "../SDL_sysvideo.h"
#include "../SDL_egl_c.h"

/* Mali */
#include "SDL_maliopengles.h"
#include "SDL_malivideo.h"

/* EGL implementation of SDL OpenGL support */

int MALI_GLES_LoadLibrary(SDL_VideoDevice *_this, const char *path)
{
    return SDL_EGL_LoadLibrary(_this, path, EGL_DEFAULT_DISPLAY, 0);
}

SDL_EGL_CreateContext_impl(MALI)
SDL_EGL_SwapWindow_impl(MALI)
SDL_EGL_MakeCurrent_impl(MALI)

#endif /* SDL_VIDEO_DRIVER_MALI && SDL_VIDEO_OPENGL_EGL */

/* vi: set ts=4 sw=4 expandtab: */
