#include <rl_sound.h>
#include <rl_memory.h>
#include <rl_config.h>

#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <math.h>

#include <rl_endian.inl>

/*---------------------------------------------------------------------------*/
#ifdef RL_OGG_VORBIS

#define STB_VORBIS_NO_CRT
#define STB_VORBIS_NO_STDIO
#define STB_VORBIS_NO_PUSHDATA_API

#define assert( x )

#if rl_malloc != malloc
#define malloc  rl_malloc
#define realloc rl_realloc
#define free    rl_free
#endif

// #define pow     pow
// #define floor   floor
// #define alloca( a ) 0

#ifdef __MINGW32__
#undef __forceinline
#define __forceinline __inline__ __attribute__((__always_inline__,__gnu_inline__))
#endif

#include "external/stb_vorbis.c"

#endif
/*---------------------------------------------------------------------------*/

typedef struct
{
  rl_voice_t     voice;
  rl_soundstop_t stop_cb;
  
  int position;
  int repeat;
}
voice_t;

static int16_t audio_buffer[ RL_SAMPLES_PER_FRAME * 2 ];
static voice_t voices[ RL_MAX_VOICES ];

#ifdef RL_OGG_VORBIS
typedef struct
{
  voice_t voice;
  
  stb_vorbis*      stream;
  stb_vorbis_alloc alloc;
  int16_t          pcm[ 4096 ];
  int              available;
}
oggvoice_t;

static oggvoice_t ogg_voice;
#endif

void rl_sound_init( void )
{
  for ( int i = 0; i < RL_MAX_VOICES; i++ )
  {
    voices[ i ].voice.sound = NULL;
  }
  
#ifdef RL_OGG_VORBIS
  ogg_voice.stream = NULL;
#endif
}

void rl_sound_done( void )
{
#ifdef RL_OGG_VORBIS
  if ( ogg_voice.stream )
  {
    stb_vorbis_close( ogg_voice.stream );
    rl_free( ogg_voice.alloc.alloc_buffer );
  }
#endif
}

int rl_sound_create( rl_sound_t* sound, const rl_snddata_t* snddata )
{
  size_t size;
  int stereo;
  const void* data = rl_snddata_encode( &size, &stereo, snddata );
  
  if ( data )
  {
    sound->samples = size / 2;
    sound->stereo = stereo;
    sound->pcm = (const int16_t*)data;
    
    return 0;
  }
  
  return -1;
}

rl_voice_t* rl_sound_play( const rl_sound_t* sound, int repeat, rl_soundstop_t stop_cb )
{
  voice_t* restrict voice = voices;
  const voice_t* restrict end = voices + RL_MAX_VOICES;
  
  do
  {
    if ( !voice->voice.sound )
    {
      voice->voice.sound = sound;
      
      voice->stop_cb  = stop_cb;
      voice->position = 0;
      voice->repeat   = repeat;
      
      return &voice->voice;
    }
    
    voice++;
  }
  while ( voice < end );
  
  return NULL;
}

void rl_sound_stop( rl_voice_t* voice_ )
{
  voice_t* voice = (voice_t*)voice_;
  
  if ( voice->stop_cb && ( voice->voice.sound || ogg_voice.stream ) )
  {
    voice->stop_cb( voice_, RL_SOUND_STOPPED );
  }
  
  if ( voice_ == &ogg_voice.voice.voice && ogg_voice.stream )
  {
    stb_vorbis_close( ogg_voice.stream );
    rl_free( ogg_voice.alloc.alloc_buffer );
    
    ogg_voice.stream = NULL;
  }
  
  voice->voice.sound = NULL;
}

void rl_sound_stop_all( void )
{
  voice_t* restrict voice = voices;
  const voice_t* restrict end = voices + RL_MAX_VOICES;
  
  while ( voice < end )
  {
    if ( voice->stop_cb && voice->voice.sound )
    {
      voice->stop_cb( &voice->voice, RL_SOUND_STOPPED );
    }
    
    voice->voice.sound = NULL;
    voice++;
  }
}

#ifdef RL_OGG_VORBIS
rl_voice_t* rl_sound_play_ogg( const void* data, size_t size, int repeat, rl_soundstop_t stop_cb )
{
  if ( !ogg_voice.stream )
  {
    // This scheme is failing with a SIGSEGV when the buffer size reaches 128Kb
    // so we just allocate 256Kb for now which seems to work.
    
    /*
    ogg_alloc.alloc_buffer = NULL;
    ogg_alloc.alloc_buffer_length_in_bytes = 0;
    
    int res;
    
    do
    {
      ogg_alloc.alloc_buffer_length_in_bytes += RL_OGG_INCREMENT;
      
      void* new_buffer = rl_realloc( ogg_alloc.alloc_buffer, ogg_alloc.alloc_buffer_length_in_bytes );
      
      if ( !new_buffer )
      {
        rl_free( ogg_alloc.alloc_buffer );
        return -1;
      }
      
      ogg_alloc.alloc_buffer = (char*)new_buffer;
      ogg_stream = stb_vorbis_open_memory( (const unsigned char*)data, size, &res, &ogg_alloc );
    }
    while ( !ogg_stream && res == VORBIS_outofmem );
    */
    
    ogg_voice.alloc.alloc_buffer = (char*)rl_malloc( 256 * 1024 );
    
    if ( ogg_voice.alloc.alloc_buffer )
    {
      ogg_voice.alloc.alloc_buffer_length_in_bytes = 256 * 1024;
      
      int res;
      ogg_voice.stream = stb_vorbis_open_memory( (const unsigned char*)data, size, &res, &ogg_voice.alloc );
      
      if ( ogg_voice.stream )
      {
        ogg_voice.voice.position = ogg_voice.available = 0;
        
        ogg_voice.voice.repeat  = repeat;
        ogg_voice.voice.stop_cb = stop_cb;
        
        return &ogg_voice.voice.voice;
      }
    
      rl_free( ogg_voice.alloc.alloc_buffer );
    }
  }
  
  ogg_voice.stream = NULL;
  return NULL;
}

static void ogg_mix( int32_t* buffer )
{
  if ( ogg_voice.stream )
  {
    int buf_free = RL_SAMPLES_PER_FRAME * 2;
    
    if ( ogg_voice.voice.position == ogg_voice.available )
    {
    again:
      ogg_voice.available = stb_vorbis_get_frame_short_interleaved( ogg_voice.stream, 2, ogg_voice.pcm, sizeof( ogg_voice.pcm ) / sizeof( ogg_voice.pcm[ 0 ] ) );
      
      if ( !ogg_voice.available )
      {
        if ( ogg_voice.voice.repeat )
        {
          if ( ogg_voice.voice.stop_cb )
          {
            ogg_voice.voice.stop_cb( &ogg_voice.voice.voice, RL_SOUND_REPEATED );
          }
          
          stb_vorbis_seek_start( ogg_voice.stream );
          goto again;
        }
        else
        {
          if ( ogg_voice.voice.stop_cb )
          {
            ogg_voice.voice.stop_cb( &ogg_voice.voice.voice, RL_SOUND_FINISHED );
          }
          
          stb_vorbis_close( ogg_voice.stream );
          ogg_voice.stream = NULL;
          return;
        }
      }
      
      ogg_voice.available *= 2;
      ogg_voice.voice.position   = 0;
    }
    
    const int16_t* pcm = ogg_voice.pcm + ogg_voice.voice.position;
    
    if ( ogg_voice.available < buf_free )
    {
      for ( int i = ogg_voice.available; i != 0; --i )
      {
        *buffer++ += *pcm++;
      }
      
      buf_free -= ogg_voice.available;
      goto again;
    }
    else
    {
      for ( int i = buf_free; i != 0; --i )
      {
        *buffer++ += *pcm++;
      }
      
      ogg_voice.voice.position += buf_free;
      ogg_voice.available      -= buf_free;
    }
  }
}
#endif /* RL_OGG_VORBIS */

static void mix( int32_t* buffer, voice_t* voice )
{
  int buf_free = RL_SAMPLES_PER_FRAME * 2;
  const rl_sound_t* sound = voice->voice.sound;
  
again:
  if ( sound->stereo )
  {
    int pcm_available = sound->samples - voice->position;
    const int16_t* pcm = sound->pcm + voice->position;
    
    if ( pcm_available < buf_free )
    {
      for ( int i = pcm_available; i != 0; --i )
      {
        *buffer++ += *pcm++;
      }
      
      if ( voice->repeat )
      {
        if ( voice->stop_cb )
        {
          voice->stop_cb( &voice->voice, RL_SOUND_REPEATED );
        }
        
        buf_free -= pcm_available;
        voice->position = 0;
        goto again;
      }
      
      if ( voice->stop_cb )
      {
        voice->stop_cb( &voice->voice, RL_SOUND_FINISHED );
      }
      
      voice->voice.sound = NULL;
    }
    else
    {
      for ( int i = buf_free; i != 0; --i )
      {
        *buffer++ += *pcm++;
      }
      
      voice->position += buf_free;
    }
  }
  else
  {
    int pcm_available = sound->samples - voice->position;
    const int16_t* pcm = sound->pcm + voice->position;
    
    buf_free /= 2;
    
    if ( pcm_available < buf_free )
    {
      for ( int i = pcm_available; i != 0; --i )
      {
        *buffer++ += *pcm;
        *buffer++ += *pcm++;
      }
      
      if ( voice->repeat )
      {
        if ( voice->stop_cb )
        {
          voice->stop_cb( &voice->voice, RL_SOUND_REPEATED );
        }
        
        buf_free -= pcm_available;
        voice->position = 0;
        goto again;
      }
      
      if ( voice->stop_cb )
      {
        voice->stop_cb( &voice->voice, RL_SOUND_FINISHED );
      }
      
      voice->voice.sound = NULL;
    }
    else
    {
      for ( int i = buf_free; i != 0; --i )
      {
        *buffer++ += *pcm;
        *buffer++ += *pcm++;
      }
      
      voice->position += buf_free;
    }
  }
}

const int16_t* rl_sound_mix( void )
{
  int32_t buffer[ RL_SAMPLES_PER_FRAME * 2 ];
  memset( buffer, 0, sizeof( buffer ) );
  
  voice_t* restrict voice = voices;
  const voice_t* restrict end   = voices + RL_MAX_VOICES;
  
  do
  {
    if ( voice->voice.sound )
    {
      mix( buffer, voice );
    }
    
    voice++;
  }
  while ( voice < end );
  
#ifdef RL_OGG_VORBIS
  ogg_mix( buffer );
#endif
  
  for ( int i = 0; i < RL_SAMPLES_PER_FRAME * 2; i++ )
  {
    int32_t sample = buffer[ i ];
    audio_buffer[ i ] = sample < -32768 ? -32768 : sample > 32767 ? 32767 : sample;
  }
  
  return audio_buffer;
}
