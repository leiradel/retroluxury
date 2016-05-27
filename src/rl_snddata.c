#include <rl_snddata.h>
#include <rl_memory.h>

#include <stdint.h>
#include <string.h>

#include <rl_endian.inl>

typedef struct
{
  /* header */
  char     chunkid[ 4 ];
  uint32_t chunksize;
  char     format[ 4 ];
  /* fmt */
  char     subchunk1id[ 4 ];
  uint32_t subchunk1size;
  uint16_t audioformat;
  uint16_t numchannels;
  uint32_t samplerate;
  uint32_t byterate;
  uint16_t blockalign;
  uint16_t bitspersample;
  /* data */
  char     subchunk2id[ 4 ];
  uint32_t subchunk2size;
  uint8_t  data[ 0 ];
  
}
wave_t;

int rl_snddata_create( rl_snddata_t* snddata, const void* data, size_t size )
{
  const wave_t* wave = (const wave_t*)data;
  
  if ( wave->chunkid[ 0 ] != 'R' || wave->chunkid[ 1 ] != 'I' || wave->chunkid[ 2 ] != 'F' || wave->chunkid[ 3 ] != 'F' )
  {
    return -1;
  }
  
  if ( wave->format[ 0 ] != 'W' || wave->format[ 1 ] != 'A' || wave->format[ 2 ] != 'V' || wave->format[ 3 ] != 'E' )
  {
    return -1;
  }
  
  if ( wave->subchunk1id[ 0 ] != 'f' || wave->subchunk1id[ 1 ] != 'm' || wave->subchunk1id[ 2 ] != 't' || wave->subchunk1id[ 3 ] != ' ' )
  {
    return -1;
  }
  
  if ( wave->subchunk2id[ 0 ] != 'd' || wave->subchunk2id[ 1 ] != 'a' || wave->subchunk2id[ 2 ] != 't' || wave->subchunk2id[ 3 ] != 'a' )
  {
    return -1;
  }
  
  snddata->bps = le16( wave->bitspersample );
  snddata->channels = le16( wave->numchannels );
  snddata->freq = le32( wave->samplerate );
  
  if ( le16( wave->audioformat ) != 1 || snddata->freq != 44100 )
  {
    return -1;
  }
  
  if ( snddata->channels != 1 && snddata->channels != 2 )
  {
    return -1;
  }
  
  if ( snddata->bps != 8 && snddata->bps != 16 )
  {
    return -1;
  }
  
  snddata->size = le32( wave->subchunk2size );
  snddata->samples = rl_malloc( snddata->size );
  
  if ( snddata->samples )
  {
    memcpy( (void*)snddata->samples, (void*)wave->data, snddata->size );
    return 0;
  }
  
  return -1;
}

const void* rl_snddata_encode( size_t* size, int* stereo, const rl_snddata_t* snddata )
{
  *size = snddata->size;
  *stereo = snddata->channels == 2;
  
  if ( snddata->bps == 8 )
  {
    int16_t* samples = (int16_t*)rl_malloc( snddata->size * 2 );
    
    if ( samples )
    {
      const void* res = (void*)samples;
      const uint8_t* begin = (const uint8_t*)snddata->samples;
      const uint8_t* end = begin + snddata->size;
      
      while ( begin < end )
      {
        int sample = *begin++;
        *samples++ = sample * 65792 / 256 - 32768;
      }
      
      return res;
    }
  }
  else
  {
    int16_t* samples = (int16_t*)rl_malloc( snddata->size );
    
    if ( samples )
    {
      const void* res = (void*)samples;
      const int16_t* begin = (const int16_t*)snddata->samples;
      const int16_t* end = begin + snddata->size / 2;
      
      while ( begin < end )
      {
        *samples++ = le16( *begin++ );
      }
      
      return res;
    }
  }
  
  return NULL;
}
