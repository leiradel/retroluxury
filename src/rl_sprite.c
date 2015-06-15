#include <rl_sprite.h>
#include <rl_memory.h>
#include <rl_config.h>

#include <stdlib.h>

typedef union item_t item_t;

union item_t
{
  rl_sprite_t sprite;
  item_t*     next;
};

typedef struct
{
  rl_sprite_t* sprite;
  uint16_t*    bg;
}
spt_t;

static item_t  items[ RL_MAX_SPRITES ];
static item_t* free_list;
static spt_t   sprites[ RL_MAX_SPRITES + 1 ];
static int     num_sprites, num_visible;
static int     x0, y0;

static uint16_t  saved_backgrnd[ RL_BG_SAVE_SIZE ];
static uint16_t* saved_ptr;

static const rl_sprite_t guard =
{
#if RL_USERDATA_COUNT <= 1
  0,
#elif RL_USERDATA_COUNT <= 2
  0,
#elif RL_USERDATA_COUNT <= 3
  0,
#elif RL_USERDATA_COUNT <= 4
  0,
#elif RL_USERDATA_COUNT <= 5
  0,
#elif RL_USERDATA_COUNT <= 6
  0,
#elif RL_USERDATA_COUNT <= 7
  0,
#elif RL_USERDATA_COUNT <= 8
  0,
#else
#error add more comparisons (or: do you really need that many userdata?)
#endif
  0,
  RL_SPRITE_UNUSED,
  0,
  0,
  NULL
};

void rl_sprite_init( void )
{
  for ( int i = 0; i < RL_MAX_SPRITES - 1; i++ )
  {
    items[ i ].next = items + i + 1;
  }
  
  items[ RL_MAX_SPRITES - 1 ].next = NULL;
  free_list = items;
  
  num_sprites = num_visible = 0;
  x0 = y0 = 0;
}

rl_sprite_t* rl_sprite_create( void )
{
  if ( num_sprites < RL_MAX_SPRITES )
  {
    rl_sprite_t* sprite = &free_list->sprite;
    free_list = free_list->next;
    
    sprites[ num_sprites++ ].sprite = sprite;
    
    sprite->layer = sprite->flags = 0;
    sprite->x = sprite->y = 0;
    sprite->image = NULL;
    
    return sprite;
  }
  
  return NULL;
}

void rl_sprites_translate( int x, int y )
{
  x0 = x;
  y0 = y;
}

static int compare( const void* e1, const void* e2 )
{
  const spt_t* s1 = (const spt_t*)e1;
  const spt_t* s2 = (const spt_t*)e2;
  
  int32_t c1 = s1->sprite->flags;
  int32_t c2 = s2->sprite->flags;
  
  c1 = c1 << 16 | s1->sprite->layer;
  c2 = c2 << 16 | s2->sprite->layer;
  
  return c1 - c2;
}

void rl_sprites_blit_nobg( void )
{
  spt_t* sptptr = sprites;
  const spt_t* endptr = sprites + num_sprites;
  
  if ( sptptr < endptr )
  {
    do
    {
      sptptr->sprite->flags &= ~RL_SPRITE_TEMP_INV;
      sptptr->sprite->flags |= sptptr->sprite->image == NULL;
      sptptr++;
    }
    while ( sptptr < endptr );
  }
  
  qsort( (void*)sprites, num_sprites, sizeof( spt_t ), compare );
  sprites[ num_sprites ].sprite = &guard;
  
  sptptr = sprites;
  
  /* Iterate over active and visible sprites and blit them */
  /* flags & 0x0007U == 0 */
  if ( sptptr->sprite->flags == 0 )
  {
    do
    {
      rl_image_blit_nobg( sptptr->sprite->image, x0 + sptptr->sprite->x, y0 + sptptr->sprite->y );
      sptptr++;
    }
    while ( sptptr->sprite->flags == 0 );
  }
  
  num_visible = sptptr - sprites;
  
  /* Jump over active but invisible sprites */
  /* flags & 0x0004U == 0x0000U */
  if ( ( sptptr->sprite->flags & RL_SPRITE_UNUSED ) == 0 )
  {
    do
    {
      sptptr++;
    }
    while ( ( sptptr->sprite->flags & RL_SPRITE_UNUSED ) == 0 );
  }
  
  int new_num_sprites = sptptr - sprites;
  
  /* Iterate over unused sprites and free them */
  /* flags & 0x0004U == 0x0004U */
  endptr = sprites + num_sprites;
  
  if ( sptptr < endptr )
  {
    do
    {
      item_t* item = (item_t*)sptptr->sprite;
      item->next = free_list;
      free_list = item;
    }
    while ( sptptr < endptr );
  }
  
  num_sprites = new_num_sprites;
}

void rl_sprites_blit( void )
{
  spt_t* sptptr = sprites;
  const spt_t* endptr = sprites + num_sprites;
  
  if ( sptptr < endptr )
  {
    do
    {
      sptptr->sprite->flags &= ~RL_SPRITE_TEMP_INV;
      sptptr->sprite->flags |= sptptr->sprite->image == NULL;
      sptptr++;
    }
    while ( sptptr < endptr );
  }
  
  qsort( (void*)sprites, num_sprites, sizeof( spt_t ), compare );
  sprites[ num_sprites ].sprite = &guard;
  
  sptptr = sprites;
  saved_ptr = saved_backgrnd;
  
  /* Iterate over active and visible sprites and blit them */
  /* flags & 0x0007U == 0 */
  if ( sptptr->sprite->flags == 0 )
  {
    do
    {
      sptptr->bg = saved_ptr;
      saved_ptr = rl_image_blit( sptptr->sprite->image, x0 + sptptr->sprite->x, y0 + sptptr->sprite->y, saved_ptr );
      sptptr++;
    }
    while ( sptptr->sprite->flags == 0 );
  }
  
  num_visible = sptptr - sprites;
  
  /* Jump over active but invisible sprites */
  /* flags & 0x0004U == 0x0000U */
  if ( ( sptptr->sprite->flags & RL_SPRITE_UNUSED ) == 0 )
  {
    do
    {
      sptptr++;
    }
    while ( ( sptptr->sprite->flags & RL_SPRITE_UNUSED ) == 0 );
  }
  
  int new_num_sprites = sptptr - sprites;
  
  /* Iterate over unused sprites and free them */
  /* flags & 0x0004U == 0x0004U */
  endptr = sprites + num_sprites;
  
  if ( sptptr < endptr )
  {
    do
    {
      item_t* item = (item_t*)sptptr->sprite;
      item->next = free_list;
      free_list = item;
    }
    while ( sptptr < endptr );
  }
  
  num_sprites = new_num_sprites;
}

void rl_sprites_unblit( void )
{
  spt_t* sptptr = sprites + num_visible - 1;
  
  /* Unblit the sprites in reverse order */
  if ( sptptr >= sprites )
  {
    do
    {
      rl_image_unblit( sptptr->sprite->image, x0 + sptptr->sprite->x, y0 + sptptr->sprite->y, sptptr->bg );
      sptptr--;
    }
    while ( sptptr >= sprites );
  }
}
