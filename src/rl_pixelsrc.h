#ifndef RL_PIXELSRC_H
#define RL_PIXELSRC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <rl_userdata.h>

#include <stdint.h>
#include <stddef.h>

typedef struct rl_pixelsrc_t rl_pixelsrc_t;

struct rl_pixelsrc_t
{
  rl_userdata_t ud;
  
  int width;  /* the image width */
  int height; /* the image height */
  int pitch;  /* how many pixels to go down to the next line */
  
  const uint32_t* abgr; /* the ABGR pixels */
  const rl_pixelsrc_t* parent; /* the parent if this pixel collection is a sub area of another pixel collection */
};

int  rl_pixelsrc_create( rl_pixelsrc_t* pixelsrc, const char* path );
int  rl_pixelsrc_sub( rl_pixelsrc_t* pixelsrc, const rl_pixelsrc_t* parent, int x0, int y0, int width, int height );
void rl_pixelsrc_destroy( const rl_pixelsrc_t* pixelsrc );

uint32_t    rl_pixelsrc_get_pixel( const rl_pixelsrc_t* pixelsrc, int x, int y );
const void* rl_pixelsrc_encode( size_t* size, const rl_pixelsrc_t* pixelsrc, int check_transp, uint16_t transparent );

#ifdef __cplusplus
}
#endif

#endif /* RL_PIXELSRC_H */
