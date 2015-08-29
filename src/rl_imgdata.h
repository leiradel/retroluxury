#ifndef RL_IMGDATA_H
#define RL_IMGDATA_H

#include <rl_userdata.h>

#include <stdint.h>
#include <stddef.h>

typedef struct
{
  rl_userdata_t ud;
  
  int bps;       /* bits per sample */
  int channels;  /* 1=mono, 2=stereo */
  int frequency; /* how many pixels to go down to the next line */
  
  const void* samples;
}
rl_snddata_t;

const rl_snddata_t* rl_imgdata_create( const void* data, size_t size );
const rl_imgdata_t* rl_imgdata_sub( const rl_imgdata_t* parent, int x0, int y0, int width, int height );
void                rl_imgdata_destroy( const rl_imgdata_t* imgdata );

uint32_t    rl_imgdata_get_pixel( const rl_imgdata_t* imgdata, int x, int y );
const void* rl_imgdata_encode( size_t* size, const rl_imgdata_t* imgdata, int check_transp, uint16_t transparent );

#endif /* RL_IMGDATA_H */
