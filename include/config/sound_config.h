/*
 * sound_config.h - Compile-time sound backend selection
 */

#pragma once

#include "SoundManager/SoundBackend.h"

#ifndef DEFAULT_SOUND_BACKEND
#define DEFAULT_SOUND_BACKEND kSoundBackendHDA
#endif

