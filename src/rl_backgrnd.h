#ifndef RL_BACKGRND_H
#define RL_BACKGRND_H

#include <stdint.h>

int  rl_backgrnd_create( int width, int height );
void rl_backgrnd_destroy( void );

void      rl_backgrnd_clear( uint16_t color );
void      rl_backgrnd_scroll( int dx, int dy );
uint16_t* rl_backgrnd_fb( int* width, int* height );

uint16_t* rl_backgrnd_get_bgptr( void );
void      rl_backgrnd_set_bgptr( uint16_t* bg_ptr );

#endif /* RL_BACKGRND_H */
