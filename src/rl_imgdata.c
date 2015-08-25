#include <rl_imgdata.h>
#include <rl_memory.h>

#include <rl_endian.c>

/*---------------------------------------------------------------------------*/
/* stb_image config and inclusion */

#define STBI_ASSERT( x )
#define STBI_MALLOC rl_malloc
#define STBI_REALLOC rl_realloc
#define STBI_FREE rl_free

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
/*---------------------------------------------------------------------------*/

const rl_imagedata_t* rl_imagedata_create( const void* data, size_t size )
{
  int width, height;
  uint32_t* abgr = (uint32_t*)stbi_load_from_memory( data, size, &width, &height, NULL, STBI_rgb_alpha );
  
  if ( abgr )
  {
    rl_imagedata_t* imagedata = (rl_imagedata_t*)rl_malloc( sizeof( rl_imagedata_t ) );
    
    if ( imagedata )
    {
      imagedata->width = width;
      imagedata->height = height;
      imagedata->pitch = width;
      imagedata->abgr = abgr;
      imagedata->parent = NULL;
      
      return imagedata;
    }
    
    rl_free( abgr );
  }
  
  return NULL;
}

const rl_imagedata_t* rl_imagedata_sub( const rl_imagedata_t* parent, int x0, int y0, int width, int height )
{
  rl_imagedata_t* imagedata = (rl_imagedata_t*)rl_malloc( sizeof( rl_imagedata_t ) );
  
  if ( imagedata )
  {
    imagedata->width = width;
    imagedata->height = height;
    imagedata->pitch = parent->pitch;
    imagedata->abgr = parent->abgr + y0 * parent->pitch + x0;
    imagedata->parent = parent;
    
    return imagedata;
  }
  
  return NULL;
}

void rl_imagedata_destroy( const rl_imagedata_t* imagedata )
{
  if ( !imagedata->parent )
  {
    rl_free( (void*)imagedata->abgr );
  }
  
  rl_free( (void*)imagedata );
}

static inline uint16_t rgb24_to_rgb16( int r, int g, int b )
{
  r >>= 3;
  g >>= 2;
  b >>= 3;
  
  return ( r << 11 ) | ( g << 5 ) | b;
}

static inline uint16_t get_pixel( int* alpha, const rl_imagedata_t* imagedata, int x, int y, int check_transp, uint16_t transparent )
{
  uint32_t abgr32 = imagedata->abgr[ y * imagedata->pitch + x ];
  int      a = abgr32 >> 24;
  int      b = ( abgr32 >> 16 ) & 255;
  int      g = ( abgr32 >> 8 ) & 255;
  int      r = abgr32 & 255;
  uint16_t rgb16 = rgb24_to_rgb16( r, g, b );
  
  if ( check_transp && rgb16 == transparent )
  {
    a = rgb16 = 0;
  }
  
  if ( a >= 0 && a <= 31 )
  {
    a = 0; /* transparent */
  }
  else if ( a >= 32 && a <= 95 )
  {
    a = 1; /* 25% */
  }
  else if ( a >= 96 && a <= 159 )
  {
    a = 2; /* 50% */
  }
  else if ( a >= 160 && a <= 223 )
  {
    a = 3; /* 75% */
  }
  else
  {
    a = 4; /* opaque */
  }
  
  *alpha = a;
  return rgb16;
}

static size_t rle_row( uint16_t* rle, int* bgcount, const rl_imagedata_t* imagedata, int y, int limit, int check_transp, uint16_t transparent )
{
  int width = imagedata->width;
  int num_cols = ( width + ( limit - 1 ) ) / limit;
  int dryrun = !rle;
  
  if ( dryrun )
  {
    rle = (uint16_t*)&dryrun;
  }
  
  uint16_t* start = rle;
  uint16_t* cols = rle;
  rle += num_cols;
  
  for ( int xx = 0; xx < width; xx += limit )
  {
    if ( !dryrun )
    {
      *cols++ = ne16( rle - start );
    }
    
    int       x = xx;
    uint16_t* runs = rle++;
    
    if ( !dryrun )
    {
      *runs = 0;
    }
    
    while ( x < ( xx + limit ) && x < width )
    {
      int a;
      get_pixel( &a, imagedata, x, y, check_transp, transparent );
      
      int count = 1;
      int xsave = x++;
      
      while ( x < ( xx + limit ) && x < width )
      {
        int aa;
        get_pixel( &aa, imagedata, x, y, check_transp, transparent );
        
        if ( aa != a )
        {
          break;
        }
        
        count++, x++;
      }
      
      if ( !dryrun )
      {
        *rle = ne16( ( a << 13 ) | count );
        ( *runs )++;
      }
      
      rle++;
      
      if ( a )
      {
        for ( int i = 0; i < count; i++ )
        {
          uint16_t rgb16 = get_pixel( &a, imagedata, xsave + i, y, check_transp, transparent );
          ( *bgcount )++;
          
          if ( !dryrun )
          {
            *rle = ne16( rgb16 );
          }
          
          rle++;
        }
      }
    }
    
    if ( !dryrun )
    {
      *runs = ne16( *runs );
    }
  }
  
  return ( rle - start ) * 2;
}

const void* rl_imagedata_rle_encode( size_t* size, const rl_imagedata_t* imagedata, int limit, int check_transp, uint16_t transparent )
{
  int    width = imagedata->width;
  int    height = imagedata->height;
  int    bgcount = 0;
  size_t total = 0;
  
  if ( !limit )
  {
    limit = 1000000;
  }
  
  for ( int y = 0; y < height; y++ )
  {
    total += rle_row( NULL, &bgcount, imagedata, y, limit, check_transp, transparent );
  }
  
  void* rle = rl_malloc( total + ( 2 * sizeof( uint16_t ) + sizeof( uint32_t ) ) + height * sizeof( uint32_t ) );
  
  if ( rle )
  {
    union
    {
      void*     restrict v;
      uint8_t*  restrict u8;
      uint16_t* restrict u16;
      uint32_t* restrict u32;
    }
    ptr;
    
    ptr.v = rle;
    
    *ptr.u16++ = ne16( width );
    *ptr.u16++ = ne16( height );
    *ptr.u32++ = ne32( bgcount );
    
    total = 0;
    
    for ( int y = 0; y < height; y++ )
    {
      *ptr.u32++ = ne32( total + y * sizeof( uint32_t ) );
      total += rle_row( NULL, &bgcount, imagedata, y, limit, check_transp, transparent );
    }
    
    for ( int y = 0; y < height; y++ )
    {
      ptr.u8 += rle_row( ptr.u16, &bgcount, imagedata, y, limit, check_transp, transparent );
    }
    
    *size = ptr.u8 - (uint8_t*)rle;
    return rle;
  }
  
  return NULL;
}
