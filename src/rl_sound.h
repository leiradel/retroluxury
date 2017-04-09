#ifndef RL_SOUND_H
#define RL_SOUND_H

#include <rl_userdata.h>
#include <rl_resample.h>

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

/* Types of sounds and voices. */
#define RL_SOUND_TYPE_NONE 0
#define RL_SOUND_TYPE_WAV  1
#define RL_SOUND_TYPE_OGG  2

/* Reasons passed to the stop callback. */
#define RL_SOUND_FINISHED 0
#define RL_SOUND_STOPPED  1
#define RL_SOUND_REPEATED 2

typedef struct
{
  rl_userdata_t ud;

  unsigned type;

  union
  {
    struct
    {
      /* wav */
      size_t         frames;
      const int16_t* pcm;
    }
    wav;

#ifdef RL_OGG_VORBIS
    struct
    {
      /* ogg */
      size_t      size;
      const void* data;
    }
    ogg;
#endif
  }
  types;
}
rl_sound_t;

typedef struct rl_voice_t rl_voice_t;
typedef void ( *rl_soundstop_t )( rl_voice_t* voice, int reason );
typedef struct stb_vorbis stb_vorbis;

struct rl_voice_t
{
  rl_userdata_t ud;

  unsigned          type;
  int               repeat;
  const rl_sound_t* sound;
  rl_soundstop_t    stop_cb;

  union
  {
    struct
    {
      /* wav */
      unsigned position;
    }
    wav;

#ifdef RL_OGG_VORBIS
    struct
    {
      /* ogg */
      unsigned    position;
      unsigned    samples;
      stb_vorbis* stream;
      int16_t*    buffer;
      size_t      buf_samples;
      rl_resampler_t* resampler;
    }
    ogg;
#endif
  }
  types;
};

void rl_sound_init( void );
void rl_sound_done( void );

int  rl_sound_create_wav( rl_sound_t* sound, const void* data, size_t size );
int  rl_sound_create_ogg( rl_sound_t* sound, const void* data, size_t size );
void rl_sound_destroy( const rl_sound_t* sound );

rl_voice_t* rl_sound_play( const rl_sound_t* sound, int repeat, rl_soundstop_t stop_cb );
void        rl_sound_stop( rl_voice_t* voice );

void rl_sound_stop_all( void );

const int16_t* rl_sound_mix( void );

#endif /* RL_SOUND_H */
