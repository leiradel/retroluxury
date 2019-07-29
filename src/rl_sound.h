#ifndef RL_SOUND_H
#define RL_SOUND_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define RL_SOUND_SPEECH_SAW      0
#define RL_SOUND_SPEECH_TRIANGLE 1
#define RL_SOUND_SPEECH_SIN      2
#define RL_SOUND_SPEECH_SQUARE   3
#define RL_SOUND_SPEECH_PULSE    4
#define RL_SOUND_SPEECH_NOISE    5
#define RL_SOUND_SPEECH_WARBLE   6

typedef struct
{
  unsigned base_frequency;
  float base_speed;
  float base_declination;
  int waveform;
}
rl_sound_speech_t;

#define RL_SOUND_SFXR_COIN      0
#define RL_SOUND_SFXR_LASER     1
#define RL_SOUND_SFXR_EXPLOSION 2
#define RL_SOUND_SFXR_POWERUP   3
#define RL_SOUND_SFXR_HURT      4
#define RL_SOUND_SFXR_JUMP      5
#define RL_SOUND_SFXR_BLIP      6

typedef struct
{
  void* opaque1;
  void* opaque2;
}
rl_sound_t;

void rl_sound_init( void );
void rl_sound_done( void );

/* Load an WAV, OGG, or a FLAC file. */
int rl_sound_load( rl_sound_t* sound, const char* path );
/* Stream an WAV, OGG, or a FLAC file. */
int rl_sound_stream( rl_sound_t* sound, const char* path );

/* Load an OpenMPT file. */
int rl_sound_mod_load( rl_sound_t* sound, const char* path );

/* Sets up a speech sound with default parameters. */
int rl_sound_speech( rl_sound_t* sound, const char* text );
/* Sets up a speech sound. */
int rl_sound_speech_params( rl_sound_t* sound, const char* text, const rl_sound_speech_t* params );

/* Loads SFXR sound parameters from disk. */
int rl_sound_sfxr_load( rl_sound_t* sound, const char* path );
/* Builds a SFXR sound based on type and a random seed. */
int rl_sound_sfxr( rl_sound_t* sound, int type, int seed );

/* Sets up a speech sound. */
int rl_sound_vizsn( rl_sound_t* sound, const char* text );

void rl_sound_destroy( const rl_sound_t* sound );

unsigned rl_sound_play( const rl_sound_t* sound, float volume, int repeat );
int      rl_sound_playing( unsigned voice );
void     rl_sound_stop( unsigned voice );

void rl_sound_stop_all( void );

const int16_t* rl_sound_mix( void );

#ifdef __cplusplus
}
#endif

#endif /* RL_SOUND_H */
