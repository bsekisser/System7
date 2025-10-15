#pragma once

#include "SystemTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum SoundEffectId {
    kSoundEffectStartupChime = 0,
    kSoundEffectBeep,
    kSoundEffectAlertCaution,
    kSoundEffectAlertStop,
    kSoundEffectAlertNote,
    kSoundEffectMenuSelect,
    kSoundEffectDroplet,
    kSoundEffectQuack,
    kSoundEffectWildEep,
    kSoundEffectIndigo,
    kSoundEffectCount
} SoundEffectId;

OSErr SoundEffects_Play(SoundEffectId effect);

#ifdef __cplusplus
}
#endif

