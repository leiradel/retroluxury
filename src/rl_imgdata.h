#ifndef RL_IMGDATA_H
#define RL_IMGDATA_H

#include <rl_userdata.h>

#include <stdint.h>
#include <stddef.h>

typedef struct rl_imagedata_t rl_imagedata_t;

struct rl_imagedata_t
{
  rl_userdata_t ud;
  
  int width;  /* the image width */
  int height; /* the image height */
  int pitch;  /* how many pixels to go down to the next line */
  
  const uint32_t* abgr; /* the ABGR pixels */
  const rl_imagedata_t* parent; /* the parent if this pixel collection is a sub area of another pixel collection */
};

const rl_imagedata_t* rl_imagedata_create( const void* data, size_t size );
const rl_imagedata_t* rl_imagedata_sub( const rl_imagedata_t* parent, int x0, int y0, int width, int height );
void                  rl_imagedata_destroy( const rl_imagedata_t* imgdata );

uint32_t    rl_imagedata_get_pixel( const rl_imagedata_t* imgdata, int x, int y );
const void* rl_imagedata_rle_encode( size_t* size, const rl_imagedata_t* imgdata, int check_transp, uint16_t transparent );

#endif /* RL_IMGDATA_H */
