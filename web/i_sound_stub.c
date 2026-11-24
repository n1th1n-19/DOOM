// Silent sound layer for the first browser milestone. Phase 2 replaces it
// with the SDL-audio mixer.
#include <stdio.h>

#include "doomdef.h"
#include "i_system.h"
#include "i_sound.h"
#include "w_wad.h"

void I_InitSound() { }
void I_UpdateSound(void) { }
void I_SubmitSound(void) { }
void I_ShutdownSound(void) { }
void I_SetChannels() { }
void I_SetSfxVolume(int volume) { snd_SfxVolume = volume; }

int I_GetSfxLumpNum (sfxinfo_t* sfx)
{
    char namebuf[9];
    sprintf (namebuf, "ds%s", sfx->name);
    return W_GetNumForName (namebuf);
}

int I_StartSound (int id, int vol, int sep, int pitch, int priority)
{
    return id;
}

void I_StopSound (int handle) { }
int  I_SoundIsPlaying (int handle) { return 0; }
void I_UpdateSoundParams (int handle, int vol, int sep, int pitch) { }

void I_InitMusic(void) { }
void I_ShutdownMusic(void) { }
void I_SetMusicVolume(int volume) { snd_MusicVolume = volume; }
void I_PauseSong (int handle) { }
void I_ResumeSong (int handle) { }
int  I_RegisterSong(void *data) { return 1; }
void I_PlaySong(int handle, int looping) { }
void I_StopSong(int handle) { }
void I_UnRegisterSong(int handle) { }
