#include <rl_sound.h>
#include <rl_snddata.h>
#include <rl_config.h>

#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <math.h>

#include <rl_endian.inl>

/*---------------------------------------------------------------------------*/
#ifdef RL_OGG_VORBIS

#define STB_VORBIS_NO_PUSHDATA_API
#define STB_VORBIS_NO_STDIO
#define STB_VORBIS_NO_CRT

#define assert( x )
#undef NULL

#ifdef __MINGW32__
#undef __forceinline
#define __forceinline __inline__ __attribute__((__always_inline__,__gnu_inline__))
#endif

#include "external/stb_vorbis.c"

#endif
/*---------------------------------------------------------------------------*/

#define RL_TEMP_OGG_BUFFER 8192

static int16_t    audio_buffer[ RL_SAMPLES_PER_FRAME * 2 ];
static rl_voice_t voices[ RL_MAX_VOICES ];

void rl_sound_init( void )
{
  for ( int i = 0; i < RL_MAX_VOICES; i++ )
  {
    voices[ i ].type = RL_SOUND_TYPE_NONE;
  }
}

void rl_sound_done( void )
{
  rl_sound_stop_all();
}

int rl_sound_create_wav( rl_sound_t* sound, const void* data, size_t size )
{
  rl_snddata_t snddata;
  
  if ( rl_snddata_create( &snddata, data, size ) != 0)
  {
    return -1;
  }
  
  size_t frames;
  data = rl_snddata_encode( &frames, &snddata );
  
  rl_snddata_destroy( &snddata );
  
  if ( data )
  {
    sound->type = RL_SOUND_TYPE_WAV;
    sound->types.wav.frames = frames;
    sound->types.wav.pcm = (const int16_t*)data;
    
    return 0;
  }
  
  return -1;
}

int rl_sound_create_ogg( rl_sound_t* sound, const void* data, size_t size )
{
  sound->type = RL_SOUND_TYPE_OGG;
  sound->types.ogg.size = size;
  sound->types.ogg.data = data;
  
  return 0;
}

void rl_sound_destroy( const rl_sound_t* sound )
{
  if ( sound->type == RL_SOUND_TYPE_WAV )
  {
    free( (void*)sound->types.wav.pcm );
  }
}

static int play_wav( rl_voice_t* voice, const rl_sound_t* sound )
{
  voice->types.wav.position = 0;
  return 0;
}

static int play_ogg( rl_voice_t* voice, const rl_sound_t* sound )
{
#ifdef RL_OGG_VORBIS
  int res;
  voice->types.ogg.stream = stb_vorbis_open_memory( (const uint8_t*)sound->types.ogg.data, sound->types.ogg.size, &res, NULL );

  if ( voice->types.ogg.stream == NULL )
  {
    return -1;
  }
  
  stb_vorbis_info info = stb_vorbis_get_info( voice->types.ogg.stream );
  voice->types.ogg.resampler = rl_resampler_create( info.sample_rate );
  
  if ( voice->types.ogg.resampler == NULL )
  {
    stb_vorbis_close( voice->types.ogg.stream );
    return -1;
  }
  
  voice->types.ogg.buf_samples = rl_resampler_out_samples( voice->types.ogg.resampler, RL_TEMP_OGG_BUFFER );
  voice->types.ogg.buffer = (int16_t*)malloc( voice->types.ogg.buf_samples );
  
  if ( voice->types.ogg.buffer == NULL )
  {
    rl_resampler_destroy( voice->types.ogg.resampler );
    stb_vorbis_close( voice->types.ogg.stream );
    return -1;
  }
  
  voice->types.ogg.position = 0;
  voice->types.ogg.samples = 0;
  return 0;
#else /* !RL_OGG_VORBIS */
  return -1;
#endif /* RL_OGG_VORBIS */
}

rl_voice_t* rl_sound_play( const rl_sound_t* sound, int repeat, rl_soundstop_t stop_cb )
{
  rl_voice_t* voice = voices;
  int res = -1;
  
  for ( int i = RL_MAX_VOICES; i != 0; voice++, --i )
  {
    if ( voice->type == RL_SOUND_TYPE_NONE )
    {
      if ( sound->type == RL_SOUND_TYPE_WAV )
      {
        res = play_wav( voice, sound );
      }
      else if ( sound->type == RL_SOUND_TYPE_OGG )
      {
        res = play_ogg( voice, sound );
      }
      
      break;
    }
    
    voice++;
  }

  if ( res == 0 )
  {
    voice->type    = sound->type;
    voice->repeat  = repeat;
    voice->sound   = sound;
    voice->stop_cb = stop_cb;
    
    return voice;
  }
  
  return NULL;
}

void rl_sound_stop( rl_voice_t* voice )
{
  if ( voice->stop_cb )
  {
    voice->stop_cb( voice, RL_SOUND_STOPPED );
    
    if ( voice->type == RL_SOUND_TYPE_OGG )
    {
      rl_resampler_destroy( voice->types.ogg.resampler );
      stb_vorbis_close( voice->types.ogg.stream );
    }
    
    voice->type = RL_SOUND_TYPE_NONE;
  }
}

void rl_sound_stop_all( void )
{
  rl_voice_t* voice = voices;

  for ( int i = 0; i < RL_MAX_VOICES; voice++, i++ )
  {
    if ( voice->type != RL_SOUND_TYPE_NONE )
    {
      rl_sound_stop( voice );
    }
  }
}

static void wav_mix( int32_t* buffer, rl_voice_t* voice )
{
  unsigned buf_free = RL_SAMPLES_PER_FRAME * 2;
  const rl_sound_t* sound = voice->sound;
  
again: ;
  unsigned pcm_available = sound->types.wav.frames * 2 - voice->types.wav.position;
  const int16_t* pcm = sound->types.wav.pcm + voice->types.wav.position;

  if ( pcm_available < buf_free )
  {
    for ( unsigned i = pcm_available; i != 0; --i )
    {
      *buffer++ += *pcm++;
    }
    
    if ( voice->repeat )
    {
      if ( voice->stop_cb )
      {
        voice->stop_cb( voice, RL_SOUND_REPEATED );
      }
      
      buf_free -= pcm_available;
      voice->types.wav.position = 0;
      goto again;
    }
    
    if ( voice->stop_cb )
    {
      voice->stop_cb( voice, RL_SOUND_FINISHED );
    }
    
    voice->type = RL_SOUND_TYPE_NONE;
  }
  else
  {
    for ( unsigned i = buf_free; i != 0; --i )
    {
      *buffer++ += *pcm++;
    }
    
    voice->types.wav.position += buf_free;
  }
}

static void ogg_mix( int32_t* buffer, rl_voice_t* voice )
{
#ifdef RL_OGG_VORBIS
  unsigned buf_free = RL_SAMPLES_PER_FRAME * 2;

  if ( voice->types.ogg.position == voice->types.ogg.samples )
  {
  again: ;
    int16_t temp_buffer[ RL_TEMP_OGG_BUFFER ];
    unsigned temp_samples = stb_vorbis_get_frame_short_interleaved( voice->types.ogg.stream, 2, temp_buffer, RL_TEMP_OGG_BUFFER ) * 2;

    if ( temp_samples == 0 )
    {
      if ( voice->repeat )
      {
        if ( voice->stop_cb )
        {
          voice->stop_cb( voice, RL_SOUND_REPEATED );
        }

        stb_vorbis_seek_start( voice->types.ogg.stream );
        goto again;
      }
      else
      {
        if ( voice->stop_cb )
        {
          voice->stop_cb( voice, RL_SOUND_FINISHED );
        }

        voice->type = RL_SOUND_TYPE_NONE;
        rl_resampler_destroy( voice->types.ogg.resampler );
        stb_vorbis_close( voice->types.ogg.stream );
        return;
      }
    }
    
    voice->types.ogg.position = 0;
    voice->types.ogg.samples = rl_resampler_resample( voice->types.ogg.resampler, temp_buffer, temp_samples, voice->types.ogg.buffer, voice->types.ogg.buf_samples );
  }

  int16_t* pcm = voice->types.ogg.buffer + voice->types.ogg.position;

  if ( voice->types.ogg.samples < buf_free )
  {
    for ( unsigned i = voice->types.ogg.samples; i != 0; --i )
    {
      *buffer++ += *pcm++;
    }

    buf_free -= voice->types.ogg.samples;
    goto again;
  }
  else
  {
    for (unsigned i = buf_free; i != 0; --i )
    {
      *buffer++ += *pcm++;
    }

    voice->types.ogg.position += buf_free;
    voice->types.ogg.samples  -= buf_free;
  }
#endif /* RL_OGG_VORBIS */
}

const int16_t* rl_sound_mix( void )
{
  int32_t buffer[ RL_SAMPLES_PER_FRAME * 2 ];
  memset( buffer, 0, sizeof( buffer ) );
  
  rl_voice_t* voice = voices;
  
  for ( int i = 0; i < RL_MAX_VOICES; voice++, i++ )
  {
    switch ( voice->type )
    {
    case RL_SOUND_TYPE_WAV:
      wav_mix( buffer, voice );
      break;
      
    case RL_SOUND_TYPE_OGG:
      ogg_mix( buffer, voice );
      break;
    }
  }
  
  for ( unsigned i = 0; i < RL_SAMPLES_PER_FRAME * 2; i++ )
  {
    int32_t sample = buffer[ i ];
    audio_buffer[ i ] = sample < -32768 ? -32768 : sample > 32767 ? 32767 : sample;
  }
  
  return audio_buffer;
}
