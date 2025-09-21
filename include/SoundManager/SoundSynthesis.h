/*
 * SoundSynthesis.h - Sound Synthesis and MIDI Support
 *
 * Provides sound synthesis, wave table support, and MIDI capabilities
 * for the Mac OS Sound Manager. Includes square wave, sampled, and
 * wave table synthesizers with complete MIDI integration.
 *
 * Copyright (c) 2025 - System 7.1 Portable Project
 */

#ifndef _SOUNDSYNTHESIS_H_
#define _SOUNDSYNTHESIS_H_

#include "SystemTypes.h"

/* Forward declarations */

#include "SystemTypes.h"

#include "SoundTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Synthesizer State Structure */

/* Ptr is defined in MacTypes.h */

/* Wave Table Entry */

/* Wave Table */

/* Square Wave Synthesizer State */

/* Sampled Sound Synthesizer State */

/* Wave Table Synthesizer State */

/* MIDI Voice State */

/* MIDI Synthesizer State */

/* Mixer Channel State */

/* Audio Mixer State */

/* Ptr is defined in MacTypes.h */

/* Synthesizer Management Functions */
OSErr SynthInit(SynthesizerPtr* synth, SInt16 synthType, UInt32 sampleRate);
OSErr SynthDispose(SynthesizerPtr synth);
OSErr SynthReset(SynthesizerPtr synth);

/* Square Wave Synthesizer */
OSErr SquareWaveInit(SquareWaveSynth* synth, UInt32 sampleRate);
OSErr SquareWaveSetFrequency(SquareWaveSynth* synth, UInt16 frequency);
OSErr SquareWaveSetAmplitude(SquareWaveSynth* synth, UInt16 amplitude);
OSErr SquareWaveSetTimbre(SquareWaveSynth* synth, UInt16 timbre);
OSErr SquareWaveGate(SquareWaveSynth* synth, Boolean gateOn);
UInt32 SquareWaveGenerate(SquareWaveSynth* synth, SInt16* buffer, UInt32 frameCount);

/* Sampled Sound Synthesizer */
OSErr SampledSynthInit(SampledSynth* synth, UInt32 sampleRate);
OSErr SampledSynthLoadSound(SampledSynth* synth, SoundHeader* header);
OSErr SampledSynthSetAmplitude(SampledSynth* synth, SInt16 amplitude);
OSErr SampledSynthSetRate(SampledSynth* synth, Fixed rate);
OSErr SampledSynthPlay(SampledSynth* synth, Boolean looping);
OSErr SampledSynthStop(SampledSynth* synth);
UInt32 SampledSynthGenerate(SampledSynth* synth, SInt16* buffer, UInt32 frameCount);

/* Wave Table Synthesizer */
OSErr WaveTableInit(WaveTableSynth* synth, UInt32 sampleRate);
OSErr WaveTableLoadTable(WaveTableSynth* synth, WaveTable* table);
OSErr WaveTableSetWave(WaveTableSynth* synth, UInt16 waveIndex);
OSErr WaveTableSetFrequency(WaveTableSynth* synth, UInt16 frequency);
OSErr WaveTableSetAmplitude(WaveTableSynth* synth, UInt16 amplitude);
UInt32 WaveTableGenerate(WaveTableSynth* synth, SInt16* buffer, UInt32 frameCount);

/* Wave Table Management */
OSErr WaveTableCreate(WaveTable** table, UInt16 numWaves);
OSErr WaveTableDispose(WaveTable* table);
OSErr WaveTableAddWave(WaveTable* table, UInt16 index,
                       SInt16* waveData, UInt32 length,
                       UInt32 loopStart, UInt32 loopEnd,
                       UInt8 baseFreq);

/* MIDI Synthesizer */
OSErr MIDISynthInit(MIDISynth* synth, UInt32 sampleRate, UInt16 maxVoices);
OSErr MIDISynthDispose(MIDISynth* synth);
OSErr MIDISynthLoadWaveTable(MIDISynth* synth, WaveTable* table);

/* MIDI Message Processing */
OSErr MIDISynthNoteOn(MIDISynth* synth, UInt8 channel, UInt8 note, UInt8 velocity);
OSErr MIDISynthNoteOff(MIDISynth* synth, UInt8 channel, UInt8 note, UInt8 velocity);
OSErr MIDISynthProgramChange(MIDISynth* synth, UInt8 channel, UInt8 program);
OSErr MIDISynthPitchBend(MIDISynth* synth, UInt8 channel, UInt16 bend);
OSErr MIDISynthControlChange(MIDISynth* synth, UInt8 channel, UInt8 controller, UInt8 value);
OSErr MIDISynthSystemReset(MIDISynth* synth);

/* MIDI Voice Management */
OSErr MIDISynthAllocateVoice(MIDISynth* synth, UInt8 channel, UInt8 note, MIDIVoice** voice);
OSErr MIDISynthReleaseVoice(MIDISynth* synth, MIDIVoice* voice);
UInt32 MIDISynthGenerate(MIDISynth* synth, SInt16* buffer, UInt32 frameCount);

/* Audio Mixer */
OSErr MixerInit(MixerPtr* mixer, UInt16 numChannels, UInt32 sampleRate);
OSErr MixerDispose(MixerPtr mixer);
OSErr MixerAddChannel(MixerPtr mixer, SynthesizerPtr synth, UInt16* channelIndex);
OSErr MixerRemoveChannel(MixerPtr mixer, UInt16 channelIndex);

/* Mixer Channel Control */
OSErr MixerSetChannelVolume(MixerPtr mixer, UInt16 channel, UInt16 volume);
OSErr MixerSetChannelPan(MixerPtr mixer, UInt16 channel, SInt16 pan);
OSErr MixerSetChannelMute(MixerPtr mixer, UInt16 channel, Boolean muted);
OSErr MixerSetChannelSolo(MixerPtr mixer, UInt16 channel, Boolean solo);
OSErr MixerFadeChannel(MixerPtr mixer, UInt16 channel,
                       UInt16 startVol, UInt16 endVol, UInt32 fadeTime);

/* Mixer Master Control */
OSErr MixerSetMasterVolume(MixerPtr mixer, UInt16 volume);
OSErr MixerSetMasterMute(MixerPtr mixer, Boolean muted);
UInt32 MixerProcess(MixerPtr mixer, SInt16* outputBuffer, UInt32 frameCount);

/* MIDI Constants */
#define MIDI_NOTE_OFF           0x80
#define MIDI_NOTE_ON            0x90
#define MIDI_POLY_PRESSURE      0xA0
#define MIDI_CONTROL_CHANGE     0xB0
#define MIDI_PROGRAM_CHANGE     0xC0
#define MIDI_CHANNEL_PRESSURE   0xD0
#define MIDI_PITCH_BEND         0xE0
#define MIDI_SYSTEM_EXCLUSIVE   0xF0

/* MIDI Control Change Numbers */
#define MIDI_CC_BANK_SELECT_MSB     0
#define MIDI_CC_MODULATION_WHEEL    1
#define MIDI_CC_BREATH_CONTROLLER   2
#define MIDI_CC_FOOT_CONTROLLER     4
#define MIDI_CC_PORTAMENTO_TIME     5
#define MIDI_CC_DATA_ENTRY_MSB      6
#define MIDI_CC_VOLUME              7
#define MIDI_CC_BALANCE             8
#define MIDI_CC_PAN                 10
#define MIDI_CC_EXPRESSION          11
#define MIDI_CC_BANK_SELECT_LSB     32
#define MIDI_CC_DATA_ENTRY_LSB      38
#define MIDI_CC_SUSTAIN_PEDAL       64
#define MIDI_CC_PORTAMENTO          65
#define MIDI_CC_SOSTENUTO           66
#define MIDI_CC_SOFT_PEDAL          67
#define MIDI_CC_LEGATO_FOOTSWITCH   68
#define MIDI_CC_HOLD_2              69

#include "SystemTypes.h"
#define MIDI_CC_SOUND_VARIATION     70
#define MIDI_CC_RESONANCE           71
#define MIDI_CC_SOUND_RELEASE_TIME  72
#define MIDI_CC_SOUND_ATTACK_TIME   73
#define MIDI_CC_SOUND_BRIGHTNESS    74
#define MIDI_CC_REVERB_LEVEL        91
#define MIDI_CC_TREMOLO_LEVEL       92
#define MIDI_CC_CHORUS_LEVEL        93
#define MIDI_CC_CELESTE_LEVEL       94
#define MIDI_CC_PHASER_LEVEL        95
#define MIDI_CC_ALL_SOUND_OFF       120
#define MIDI_CC_ALL_CONTROLLERS_OFF 121
#define MIDI_CC_LOCAL_KEYBOARD      122
#define MIDI_CC_ALL_NOTES_OFF       123

/* Note Frequency Table (A4 = 440 Hz) */
extern const UInt16 MIDI_NOTE_FREQUENCIES[128];

/* General MIDI Program Names */
extern const char* GM_PROGRAM_NAMES[128];

/* Envelope Phases */

/* Interpolation Modes */

/* Synthesis Utility Functions */
UInt16 NoteToFrequency(UInt8 note);
UInt8 FrequencyToNote(UInt16 frequency);
SInt16 LinearInterpolate(SInt16 sample1, SInt16 sample2, UInt32 fraction);
SInt16 CubicInterpolate(SInt16 s0, SInt16 s1, SInt16 s2, SInt16 s3, UInt32 fraction);

/* Envelope Generator Functions */
UInt16 EnvelopeProcess(MIDIVoice* voice);
void EnvelopeNoteOn(MIDIVoice* voice);
void EnvelopeNoteOff(MIDIVoice* voice);

/* Audio Processing Utilities */
void ApplyVolume(SInt16* buffer, UInt32 frameCount, UInt16 volume);
void ApplyPan(SInt16* leftBuffer, SInt16* rightBuffer, UInt32 frameCount, SInt16 pan);
void MixBuffers(SInt16* dest, const SInt16* src, UInt32 frameCount, UInt16 volume);
void ConvertSampleFormat(void* src, void* dest, UInt32 samples,
                        AudioEncodingType srcFormat, AudioEncodingType destFormat);

#ifdef __cplusplus
}
#endif

#endif /* _SOUNDSYNTHESIS_H_ */