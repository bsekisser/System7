#include "SystemTypes.h"
#include <stdlib.h>
#include <string.h>
/*
 * SoundSynthesis.c - Sound Synthesis and MIDI Implementation
 *
 * Provides complete sound synthesis capabilities including:
 * - Square wave synthesizer (Mac compatible)
 * - Sampled sound synthesizer with interpolation
 * - Wave table synthesizer with looping
 * - MIDI synthesizer with General MIDI support
 * - Envelope generation (ADSR)
 * - Audio mixing and effects
 *
 * Copyright (c) 2025 - System 7.1 Portable Project
 */

// #include "CompatibilityFix.h" // Removed
#include "SystemTypes.h"
#include "System71StdLib.h"

#include <math.h>

#include "SoundManager/SoundSynthesis.h"
#include "SoundManager/SoundTypes.h"


/* Mathematical constants */
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* MIDI Note Frequency Table (A4 = 440 Hz) */
const UInt16 MIDI_NOTE_FREQUENCIES[128] = {
    8, 9, 9, 10, 10, 11, 12, 12, 13, 14, 15, 15, 16, 17, 18, 19,
    21, 22, 23, 24, 26, 28, 29, 31, 33, 35, 37, 39, 41, 44, 46, 49,
    52, 55, 58, 62, 65, 69, 73, 78, 82, 87, 93, 98, 104, 110, 117, 123,
    131, 139, 147, 156, 165, 175, 185, 196, 208, 220, 233, 247, 262, 277, 294, 311,
    330, 349, 370, 392, 415, 440, 466, 494, 523, 554, 587, 622, 659, 698, 740, 784,
    831, 880, 932, 988, 1047, 1109, 1175, 1245, 1319, 1397, 1480, 1568, 1661, 1760, 1865, 1976,
    2093, 2217, 2349, 2489, 2637, 2794, 2960, 3136, 3322, 3520, 3729, 3951, 4186, 4435, 4699, 4978,
    5274, 5588, 5920, 6272, 6645, 7040, 7459, 7902, 8372, 8870, 9397, 9956, 10548, 11175, 11840, 12544
};

/* General MIDI Program Names */
const char* GM_PROGRAM_NAMES[128] = {
    "Acoustic Grand Piano", "Bright Acoustic Piano", "Electric Grand Piano", "Honky-tonk Piano",
    "Electric Piano 1", "Electric Piano 2", "Harpsichord", "Clavi",
    "Celesta", "Glockenspiel", "Music Box", "Vibraphone",
    "Marimba", "Xylophone", "Tubular Bells", "Dulcimer",
    "Drawbar Organ", "Percussive Organ", "Rock Organ", "Church Organ",
    "Reed Organ", "Accordion", "Harmonica", "Tango Accordion",
    "Acoustic Guitar (nylon)", "Acoustic Guitar (steel)", "Electric Guitar (jazz)", "Electric Guitar (clean)",
    "Electric Guitar (muted)", "Overdriven Guitar", "Distortion Guitar", "Guitar Harmonics",
    "Acoustic Bass", "Electric Bass (finger)", "Electric Bass (pick)", "Fretless Bass",
    "Slap Bass 1", "Slap Bass 2", "Synth Bass 1", "Synth Bass 2",
    "Violin", "Viola", "Cello", "Contrabass",
    "Tremolo Strings", "Pizzicato Strings", "Orchestral Harp", "Timpani",
    "String Ensemble 1", "String Ensemble 2", "SynthStrings 1", "SynthStrings 2",
    "Choir Aahs", "Voice Oohs", "Synth Voice", "Orchestra Hit",
    "Trumpet", "Trombone", "Tuba", "Muted Trumpet",
    "French Horn", "Brass Section", "SynthBrass 1", "SynthBrass 2",
    "Soprano Sax", "Alto Sax", "Tenor Sax", "Baritone Sax",
    "Oboe", "English Horn", "Bassoon", "Clarinet",
    "Piccolo", "Flute", "Recorder", "Pan Flute",
    "Blown Bottle", "Shakuhachi", "Whistle", "Ocarina",
    "Lead 1 (square)", "Lead 2 (sawtooth)", "Lead 3 (calliope)", "Lead 4 (chiff)",
    "Lead 5 (charang)", "Lead 6 (voice)", "Lead 7 (fifths)", "Lead 8 (bass + lead)",
    "Pad 1 (new age)", "Pad 2 (warm)", "Pad 3 (polysynth)", "Pad 4 (choir)",
    "Pad 5 (bowed)", "Pad 6 (metallic)", "Pad 7 (halo)", "Pad 8 (sweep)",
    "FX 1 (rain)", "FX 2 (soundtrack)", "FX 3 (crystal)", "FX 4 (atmosphere)",
    "FX 5 (brightness)", "FX 6 (goblins)", "FX 7 (echoes)", "FX 8 (sci-fi)",
    "Sitar", "Banjo", "Shamisen", "Koto",
    "Kalimba", "Bag pipe", "Fiddle", "Shanai",
    "Tinkle Bell", "Agogo", "Steel Drums", "Woodblock",
    "Taiko Drum", "Melodic Tom", "Synth Drum", "Reverse Cymbal",
    "Guitar Fret Noise", "Breath Noise", "Seashore", "Bird Tweet",
    "Telephone Ring", "Helicopter", "Applause", "Gunshot"
};

/*
 * Square Wave Synthesizer Implementation
 */
OSErr SquareWaveInit(SquareWaveSynth* synth, UInt32 sampleRate)
{
    if (synth == NULL) {
        return paramErr;
    }

    memset(synth, 0, sizeof(SquareWaveSynth));

    (synth)->synthType = squareWaveSynth;
    (synth)->active = true;
    (synth)->busy = false;
    (synth)->sampleRate = sampleRate;
    (synth)->channels = 1;
    (synth)->bitDepth = 16;

    synth->frequency = 440; /* A4 */
    synth->amplitude = 32767; /* Full volume */
    synth->timbre = 128; /* 50% duty cycle */
    synth->phase = 0;
    synth->phaseIncrement = (UInt32)(((uint64_t)synth->frequency << 32) / sampleRate);
    synth->gate = false;

    return noErr;
}

OSErr SquareWaveSetFrequency(SquareWaveSynth* synth, UInt16 frequency)
{
    if (synth == NULL) {
        return paramErr;
    }

    synth->frequency = frequency;
    synth->phaseIncrement = (UInt32)(((uint64_t)frequency << 32) / (synth)->sampleRate);

    return noErr;
}

OSErr SquareWaveSetAmplitude(SquareWaveSynth* synth, UInt16 amplitude)
{
    if (synth == NULL) {
        return paramErr;
    }

    /* Scale amplitude from Mac OS range (0-255) to 16-bit */
    synth->amplitude = (amplitude * 32767) / 255;

    return noErr;
}

OSErr SquareWaveSetTimbre(SquareWaveSynth* synth, UInt16 timbre)
{
    if (synth == NULL) {
        return paramErr;
    }

    /* Timbre controls duty cycle (0-255) */
    synth->timbre = timbre;

    return noErr;
}

OSErr SquareWaveGate(SquareWaveSynth* synth, Boolean gateOn)
{
    if (synth == NULL) {
        return paramErr;
    }

    synth->gate = gateOn;
    if (!gateOn) {
        synth->phase = 0; /* Reset phase when gate off */
    }

    return noErr;
}

UInt32 SquareWaveGenerate(SquareWaveSynth* synth, SInt16* buffer, UInt32 frameCount)
{
    UInt32 i;
    SInt16 sample;
    UInt32 dutyThreshold;

    if (synth == NULL || buffer == NULL || !synth->gate) {
        if (buffer != NULL) {
            memset(buffer, 0, frameCount * sizeof(SInt16));
        }
        return frameCount;
    }

    dutyThreshold = ((UInt32)synth->timbre << 24) / 255; /* Convert to 32-bit phase */

    for (i = 0; i < frameCount; i++) {
        /* Generate square wave based on phase and duty cycle */
        if (synth->phase < dutyThreshold) {
            sample = synth->amplitude;
        } else {
            sample = -synth->amplitude;
        }

        buffer[i] = sample;
        synth->phase += synth->phaseIncrement;
    }

    return frameCount;
}

/*
 * Sampled Sound Synthesizer Implementation
 */
OSErr SampledSynthInit(SampledSynth* synth, UInt32 sampleRate)
{
    if (synth == NULL) {
        return paramErr;
    }

    memset(synth, 0, sizeof(SampledSynth));

    (synth)->synthType = sampledSynth;
    (synth)->active = true;
    (synth)->busy = false;
    (synth)->sampleRate = sampleRate;
    (synth)->channels = 1;
    (synth)->bitDepth = 16;

    synth->amplitude = 32767;
    synth->playbackRate = X2Fixed(1); /* Normal speed */
    synth->interpolation = INTERP_LINEAR;

    return noErr;
}

OSErr SampledSynthLoadSound(SampledSynth* synth, SoundHeader* header)
{
    if (synth == NULL || header == NULL) {
        return paramErr;
    }

    synth->soundHeader = header;
    synth->sampleData = header->sampleArea;
    synth->sampleLength = header->length;
    synth->sampleRate = Fixed2X(header->sampleRate);
    synth->loopStart = header->loopStart;
    synth->loopEnd = header->loopEnd;
    synth->looping = (header->loopEnd > header->loopStart);
    synth->encoding = header->encode;
    synth->currentPos = 0;

    return noErr;
}

OSErr SampledSynthSetAmplitude(SampledSynth* synth, SInt16 amplitude)
{
    if (synth == NULL) {
        return paramErr;
    }

    synth->amplitude = amplitude;
    return noErr;
}

OSErr SampledSynthSetRate(SampledSynth* synth, Fixed rate)
{
    if (synth == NULL) {
        return paramErr;
    }

    synth->playbackRate = rate;
    return noErr;
}

OSErr SampledSynthPlay(SampledSynth* synth, Boolean looping)
{
    if (synth == NULL || synth->sampleData == NULL) {
        return paramErr;
    }

    synth->currentPos = 0;
    synth->looping = looping;
    (synth)->busy = true;

    return noErr;
}

OSErr SampledSynthStop(SampledSynth* synth)
{
    if (synth == NULL) {
        return paramErr;
    }

    (synth)->busy = false;
    synth->currentPos = 0;

    return noErr;
}

UInt32 SampledSynthGenerate(SampledSynth* synth, SInt16* buffer, UInt32 frameCount)
{
    UInt32 i;
    UInt32 framesGenerated = 0;
    SInt16 sample;
    UInt32 sampleIndex;
    Fixed rateIncrement;

    if (synth == NULL || buffer == NULL || !(synth)->busy || synth->sampleData == NULL) {
        if (buffer != NULL) {
            memset(buffer, 0, frameCount * sizeof(SInt16));
        }
        return frameCount;
    }

    /* Calculate rate increment for sample rate conversion */
    rateIncrement = (Fixed)(((uint64_t)synth->sampleRate * synth->playbackRate) / (synth)->sampleRate);

    for (i = 0; i < frameCount; i++) {
        sampleIndex = Fixed2X(synth->currentPos);

        /* Check bounds */
        if (sampleIndex >= synth->sampleLength) {
            if (synth->looping && synth->loopEnd > synth->loopStart) {
                /* Loop back to loop start */
                synth->currentPos = X2Fixed(synth->loopStart);
                sampleIndex = synth->loopStart;
            } else {
                /* End of sample */
                (synth)->busy = false;
                break;
            }
        }

        /* Get sample based on encoding */
        switch (synth->encoding) {
            case k8BitOffsetBinaryFormat:
                sample = ((SInt16)synth->sampleData[sampleIndex] - 128) << 8;
                break;

            case k16BitBigEndianFormat:
                if (sampleIndex * 2 + 1 < synth->sampleLength) {
                    sample = (synth->sampleData[sampleIndex * 2] << 8) |
                            synth->sampleData[sampleIndex * 2 + 1];
                } else {
                    sample = 0;
                }
                break;

            default:
                sample = 0;
                break;
        }

        /* Apply amplitude scaling */
        sample = (sample * synth->amplitude) >> 15;

        buffer[i] = sample;
        synth->currentPos += rateIncrement;
        framesGenerated++;
    }

    /* Fill remaining buffer with silence if sample ended */
    for (; i < frameCount; i++) {
        buffer[i] = 0;
    }

    return frameCount;
}

/*
 * Wave Table Synthesizer Implementation
 */
OSErr WaveTableInit(WaveTableSynth* synth, UInt32 sampleRate)
{
    if (synth == NULL) {
        return paramErr;
    }

    memset(synth, 0, sizeof(WaveTableSynth));

    (synth)->synthType = waveTableSynth;
    (synth)->active = true;
    (synth)->busy = false;
    (synth)->sampleRate = sampleRate;
    (synth)->channels = 1;
    (synth)->bitDepth = 16;

    synth->frequency = 440;
    synth->amplitude = 32767;
    synth->currentPos = 0;
    synth->looping = true;

    return noErr;
}

OSErr WaveTableLoadTable(WaveTableSynth* synth, WaveTable* table)
{
    if (synth == NULL || table == NULL) {
        return paramErr;
    }

    synth->waveTable = table;
    synth->currentWave = 0;

    if (table->numWaves > 0) {
        WaveTableEntry* wave = &table->waves[0];
        synth->phaseIncrement = (UInt32)(((uint64_t)synth->frequency * wave->waveLength) / (synth)->sampleRate);
    }

    return noErr;
}

OSErr WaveTableSetWave(WaveTableSynth* synth, UInt16 waveIndex)
{
    if (synth == NULL || synth->waveTable == NULL) {
        return paramErr;
    }

    if (waveIndex >= synth->waveTable->numWaves) {
        return paramErr;
    }

    synth->currentWave = waveIndex;

    /* Recalculate phase increment */
    WaveTableEntry* wave = &synth->waveTable->waves[waveIndex];
    synth->phaseIncrement = (UInt32)(((uint64_t)synth->frequency * wave->waveLength) / (synth)->sampleRate);

    return noErr;
}

OSErr WaveTableSetFrequency(WaveTableSynth* synth, UInt16 frequency)
{
    if (synth == NULL || synth->waveTable == NULL) {
        return paramErr;
    }

    synth->frequency = frequency;

    if (synth->currentWave < synth->waveTable->numWaves) {
        WaveTableEntry* wave = &synth->waveTable->waves[synth->currentWave];
        synth->phaseIncrement = (UInt32)(((uint64_t)frequency * wave->waveLength) / (synth)->sampleRate);
    }

    return noErr;
}

OSErr WaveTableSetAmplitude(WaveTableSynth* synth, UInt16 amplitude)
{
    if (synth == NULL) {
        return paramErr;
    }

    synth->amplitude = amplitude;
    return noErr;
}

UInt32 WaveTableGenerate(WaveTableSynth* synth, SInt16* buffer, UInt32 frameCount)
{
    UInt32 i;
    SInt16 sample;
    WaveTableEntry* wave;
    UInt32 wavePos;

    if (synth == NULL || buffer == NULL || synth->waveTable == NULL ||
        synth->currentWave >= synth->waveTable->numWaves) {
        memset(buffer, 0, frameCount * sizeof(SInt16));
        return frameCount;
    }

    wave = &synth->waveTable->waves[synth->currentWave];

    for (i = 0; i < frameCount; i++) {
        wavePos = synth->currentPos / (0xFFFFFFFF / wave->waveLength);

        /* Bounds checking */
        if (wavePos >= wave->waveLength) {
            if (synth->looping && wave->looping) {
                synth->currentPos = (wave->loopStart * 0xFFFFFFFF) / wave->waveLength;
                wavePos = wave->loopStart;
            } else {
                wavePos = wave->waveLength - 1;
            }
        }

        /* Linear interpolation */
        if (wavePos < wave->waveLength - 1) {
            UInt32 fraction = (synth->currentPos % (0xFFFFFFFF / wave->waveLength)) /
                               ((0xFFFFFFFF / wave->waveLength) / 256);
            sample = LinearInterpolate(wave->waveData[wavePos],
                                     wave->waveData[wavePos + 1],
                                     fraction);
        } else {
            sample = wave->waveData[wavePos];
        }

        /* Apply amplitude */
        sample = (sample * synth->amplitude) >> 15;
        buffer[i] = sample;

        synth->currentPos += synth->phaseIncrement;
    }

    return frameCount;
}

/*
 * MIDI Synthesizer Implementation
 */
OSErr MIDISynthInit(MIDISynth* synth, UInt32 sampleRate, UInt16 maxVoices)
{
    int i;

    if (synth == NULL || maxVoices > 32) {
        return paramErr;
    }

    memset(synth, 0, sizeof(MIDISynth));

    (synth)->synthType = MIDISynthOut;
    (synth)->active = true;
    (synth)->busy = false;
    (synth)->sampleRate = sampleRate;
    (synth)->channels = 2; /* Stereo */
    (synth)->bitDepth = 16;

    synth->maxVoices = maxVoices;
    synth->activeVoices = 0;

    /* Initialize MIDI channels */
    for (i = 0; i < 16; i++) {
        synth->channelProgram[i] = 0;
        synth->channelPitchBend[i] = 8192; /* Center */
        synth->channelVolume[i] = 100;
        synth->channelPan[i] = 64; /* Center */
        synth->channelMute[i] = false;
        synth->sustainPedal[i] = false;
    }

    /* Initialize voices */
    for (i = 0; i < 32; i++) {
        synth->voices[i].active = false;
        synth->voices[i].envelopePhase = ENV_PHASE_OFF;
    }

    return noErr;
}

OSErr MIDISynthDispose(MIDISynth* synth)
{
    if (synth == NULL) {
        return paramErr;
    }

    /* Release all voices */
    for (int i = 0; i < 32; i++) {
        synth->voices[i].active = false;
    }

    (synth)->active = false;
    return noErr;
}

OSErr MIDISynthNoteOn(MIDISynth* synth, UInt8 channel, UInt8 note, UInt8 velocity)
{
    MIDIVoice* voice;
    OSErr err;

    if (synth == NULL || channel >= 16 || note >= 128 || velocity == 0) {
        return paramErr;
    }

    /* Allocate voice */
    err = MIDISynthAllocateVoice(synth, channel, note, &voice);
    if (err != noErr) {
        return err;
    }

    voice->channel = channel;
    voice->note = note;
    voice->velocity = velocity;
    voice->frequency = MIDI_NOTE_FREQUENCIES[note];
    voice->amplitude = (velocity * synth->channelVolume[channel] * 32767) / (127 * 127);
    voice->currentTime = 0;
    voice->envelopePhase = ENV_PHASE_ATTACK;

    /* Set envelope parameters (basic ADSR) */
    voice->attackTime = (synth)->sampleRate / 100; /* 10ms attack */
    voice->decayTime = (synth)->sampleRate / 50;   /* 20ms decay */
    voice->sustainLevel = (voice->amplitude * 3) / 4; /* 75% sustain */
    voice->releaseTime = (synth)->sampleRate / 10; /* 100ms release */

    voice->active = true;
    EnvelopeNoteOn(voice);

    return noErr;
}

OSErr MIDISynthNoteOff(MIDISynth* synth, UInt8 channel, UInt8 note, UInt8 velocity)
{
    int i;

    if (synth == NULL || channel >= 16 || note >= 128) {
        return paramErr;
    }

    /* Find and release matching voices */
    for (i = 0; i < synth->maxVoices; i++) {
        MIDIVoice* voice = &synth->voices[i];
        if (voice->active && voice->channel == channel && voice->note == note) {
            if (!synth->sustainPedal[channel]) {
                EnvelopeNoteOff(voice);
            }
        }
    }

    return noErr;
}

OSErr MIDISynthAllocateVoice(MIDISynth* synth, UInt8 channel, UInt8 note, MIDIVoice** voice)
{
    int i;
    int oldestVoice = -1;
    UInt32 oldestTime = 0;

    if (synth == NULL || voice == NULL) {
        return paramErr;
    }

    /* Find free voice */
    for (i = 0; i < synth->maxVoices; i++) {
        if (!synth->voices[i].active) {
            *voice = &synth->voices[i];
            return noErr;
        }

        /* Track oldest voice for stealing */
        if (synth->voices[i].currentTime > oldestTime) {
            oldestTime = synth->voices[i].currentTime;
            oldestVoice = i;
        }
    }

    /* Voice stealing - use oldest voice */
    if (oldestVoice >= 0) {
        *voice = &synth->voices[oldestVoice];
        return noErr;
    }

    return notEnoughHardwareErr;
}

UInt32 MIDISynthGenerate(MIDISynth* synth, SInt16* buffer, UInt32 frameCount)
{
    SInt16* mixBuffer;
    UInt32 i, v;
    SInt32 sample;

    if (synth == NULL || buffer == NULL) {
        memset(buffer, 0, frameCount * 2 * sizeof(SInt16));
        return frameCount;
    }

    /* Clear output buffer */
    memset(buffer, 0, frameCount * 2 * sizeof(SInt16));

    /* Mix all active voices */
    for (v = 0; v < synth->maxVoices; v++) {
        MIDIVoice* voice = &synth->voices[v];

        if (!voice->active) {
            continue;
        }

        /* Generate samples for this voice */
        for (i = 0; i < frameCount; i++) {
            /* Simple sine wave oscillator */
            sample = (SInt16)(sin(2.0 * M_PI * voice->frequency * voice->currentTime / (synth)->sampleRate) *
                             EnvelopeProcess(voice));

            /* Mix into output buffer (stereo) */
            buffer[i * 2] += sample; /* Left */
            buffer[i * 2 + 1] += sample; /* Right */

            voice->currentTime++;
        }

        /* Remove voice if envelope finished */
        if (voice->envelopePhase == ENV_PHASE_OFF) {
            voice->active = false;
            synth->activeVoices--;
        }
    }

    return frameCount;
}

/*
 * Utility Functions
 */
UInt16 NoteToFrequency(UInt8 note)
{
    if (note >= 128) {
        return 0;
    }
    return MIDI_NOTE_FREQUENCIES[note];
}

UInt8 FrequencyToNote(UInt16 frequency)
{
    int i;
    UInt16 closestFreq = MIDI_NOTE_FREQUENCIES[0];
    UInt8 closestNote = 0;

    for (i = 1; i < 128; i++) {
        if (abs((int)frequency - (int)MIDI_NOTE_FREQUENCIES[i]) <
            abs((int)frequency - (int)closestFreq)) {
            closestFreq = MIDI_NOTE_FREQUENCIES[i];
            closestNote = i;
        }
    }

    return closestNote;
}

SInt16 LinearInterpolate(SInt16 sample1, SInt16 sample2, UInt32 fraction)
{
    return sample1 + (((sample2 - sample1) * (SInt32)fraction) >> 8);
}

SInt16 CubicInterpolate(SInt16 s0, SInt16 s1, SInt16 s2, SInt16 s3, UInt32 fraction)
{
    float f = (float)fraction / 256.0f;
    float a = (float)(s3 - s2 - s0 + s1);
    float b = (float)(s0 - s1 - a);
    float c = (float)(s2 - s0);
    float d = (float)s1;

    return (SInt16)(a * f * f * f + b * f * f + c * f + d);
}

UInt16 EnvelopeProcess(MIDIVoice* voice)
{
    UInt16 amplitude = 0;

    switch (voice->envelopePhase) {
        case ENV_PHASE_ATTACK:
            if (voice->currentTime < voice->attackTime) {
                amplitude = (voice->amplitude * voice->currentTime) / voice->attackTime;
            } else {
                voice->envelopePhase = ENV_PHASE_DECAY;
                voice->currentTime = 0;
                amplitude = voice->amplitude;
            }
            break;

        case ENV_PHASE_DECAY:
            if (voice->currentTime < voice->decayTime) {
                amplitude = voice->amplitude -
                           ((voice->amplitude - voice->sustainLevel) * voice->currentTime) / voice->decayTime;
            } else {
                voice->envelopePhase = ENV_PHASE_SUSTAIN;
                amplitude = voice->sustainLevel;
            }
            break;

        case ENV_PHASE_SUSTAIN:
            amplitude = voice->sustainLevel;
            break;

        case ENV_PHASE_RELEASE:
            if (voice->currentTime < voice->releaseTime) {
                amplitude = voice->sustainLevel -
                           (voice->sustainLevel * voice->currentTime) / voice->releaseTime;
            } else {
                voice->envelopePhase = ENV_PHASE_OFF;
                voice->active = false;
                amplitude = 0;
            }
            break;

        default:
            amplitude = 0;
            break;
    }

    voice->currentAmp = amplitude;
    return amplitude;
}

void EnvelopeNoteOn(MIDIVoice* voice)
{
    voice->envelopePhase = ENV_PHASE_ATTACK;
    voice->currentTime = 0;
}

void EnvelopeNoteOff(MIDIVoice* voice)
{
    if (voice->envelopePhase != ENV_PHASE_OFF) {
        voice->envelopePhase = ENV_PHASE_RELEASE;
        voice->currentTime = 0;
    }
}
