#include <string.h>
#include <rl_backgrnd.h>
#include <rl_memory.h>
#include <rl_config.h>

static uint16_t* pixels;
static int       width, height;
static uint16_t* fb;

static uint16_t  saved_backgrnd[ RL_BG_SAVE_SIZE ];
static uint16_t* saved_ptr;

int rl_backgrnd_create( int w, int h )
{
  pixels = (uint16_t*)rl_malloc( ( ( w + RL_BACKGRND_MARGIN ) * h + RL_BACKGRND_MARGIN ) * sizeof( uint16_t ) );
  
  if ( pixels )
  {
    width  = w;
    height = h;
    fb     = (uint16_t*)pixels + RL_BACKGRND_MARGIN;
    
    saved_ptr = saved_backgrnd;
    
    return 0;
  }
  
  return -1;
}

void rl_backgrnd_destroy( void )
{
  rl_free( pixels );
}

void rl_backgrnd_clear( uint16_t color )
{
  uint16_t* pixel = fb;
  
  for ( int y = 0; y < height; y++ )
  {
    for ( int x = 0; x < width; x++ )
    {
      *pixel++ = color;
    }
    
    pixel += RL_BACKGRND_MARGIN;
  }
}

void rl_backgrnd_scroll( int dx, int dy )
{
  int       pitch  = width + RL_BACKGRND_MARGIN;
  uint16_t* dest   = fb;
  uint16_t* source = dest - dy * pitch - dx;
  int       count  = pitch * ( height - 1 ) + width;
  
  if ( dy > 0 )
  {
    source += dy * pitch;
    dest   += dy * pitch;
    count  -= dy * pitch;
  }
  
  if ( dy < 0 )
  {
    count += dy * pitch;
  }
  
  if ( dx > 0 )
  {
    source += dx;
    dest   += dx;
    count  -= dx;
  }
  
  if ( dx < 0 )
  {
    count += dx;
  }
  
  if ( count > 0 )
  {
    memmove( (void*)dest, (void*)source, count * sizeof( uint16_t ) );
  }
}

uint16_t* rl_backgrnd_fb( int* w, int* h )
{
  if ( w )
  {
    *w = width;
  }
  
  if ( h )
  {
    *h = height;
  }
  
  return fb;
}

uint16_t* rl_backgrnd_get_bgptr( void )
{
  return saved_ptr;
}

void rl_backgrnd_set_bgptr( uint16_t* bg_ptr )
{
  saved_ptr = bg_ptr;
}
