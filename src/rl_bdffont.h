#ifndef RL_BDFFONT_H
#define RL_BDFFONT_H

#include <rl_userdata.h>
#include <rl_pixelsrc.h>

#include <stdint.h>
#include <stdlib.h>

typedef struct
{
  int code;
  int dwx0, dwy0;
  int bbw, bbh, bbxoff0x, bbyoff0y, wbytes;

  const uint8_t* bits;
}
rl_bdffontchar_t;

typedef struct
{
  rl_userdata_t ud;
  
  int metrics_set, num_chars;
  rl_bdffontchar_t* chars;
}
rl_bdffont_t;

typedef int (*rl_bdffont_filter_t)( int encoding, int glyph_index, void* userdata );

int  rl_bdffont_create( rl_bdffont_t* bdffont, const char* source );
int  rl_bdffont_create_filter( rl_bdffont_t* bdffont, const char* source, rl_bdffont_filter_t filter, void* userdata );
void rl_bdffont_destroy( const rl_bdffont_t* bdffont );

void rl_bdffont_size( const rl_bdffont_t* bdffont, int* x0, int* y0, int* width, int* height, const char* text );

int  rl_bdffont_render( rl_pixelsrc_t* pixelsrc, const rl_bdffont_t* bdffont, int* x0, int* y0, const char* text, uint32_t bg_color, uint32_t fg_color );

#endif /* RL_BDFFONT_H */
