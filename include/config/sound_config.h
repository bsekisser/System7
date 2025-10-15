/*
 * sound_config.h - Compile-time sound backend selection
 */

#pragma once

#include "SoundManager/SoundBackend.h"

#ifndef DEFAULT_SOUND_BACKEND
#define DEFAULT_SOUND_BACKEND kSoundBackendHDA
#endif

/*
 * To target a different backend (for example SB16), override
 * DEFAULT_SOUND_BACKEND at build time, e.g. add
 *   -DDEFAULT_SOUND_BACKEND=kSoundBackendSB16
 * in your compiler flags.
 */
