#include <rl_sprite.h>
#include <rl_memory.h>
#include <rl_config.h>

#include <stdlib.h>

typedef struct
{
  rl_sprite_t sprite;
  uint16_t*   bg;
}
spt_t;

static spt_t     sprites[ RL_MAX_SPRITES + 1 ];
static uint16_t  saved_backgrnd[ RL_BG_SAVE_SIZE ];
static uint16_t* saved_ptr;
static int       num_sprites, num_visible;

void rl_sprite_init( void )
{
  num_sprites = num_visible = 0;
}

rl_sprite_t* rl_sprite_create( void )
{
  if ( num_sprites < RL_MAX_SPRITES )
  {
    rl_sprite_t* sprite = &sprites[ num_sprites++ ].sprite;
    
    sprite->layer = 0;
    sprite->x = sprite->y = 0;
    sprite->image = NULL;
    
    return sprite;
  }
  
  return NULL;
}

static int compare( const void* e1, const void* e2 )
{
  const spt_t* s1 = (const spt_t*)e1;
  const spt_t* s2 = (const spt_t*)e2;
  
  int32_t c1 = s1->sprite.layer | ( -( s1->sprite.image == NULL ) & RL_SPRITE_INVISIBLE );
  int32_t c2 = s2->sprite.layer | ( -( s2->sprite.image == NULL ) & RL_SPRITE_INVISIBLE );
  
  return c1 - c2;
}

void rl_sprites_begin( void )
{
  spt_t* sptptr = sprites;
  
  qsort( (void*)sprites, num_sprites, sizeof( spt_t ), compare );
  sprites[ num_sprites ].sprite.layer = RL_SPRITE_UNUSED; /* guard */
  
  saved_ptr = saved_backgrnd;
  
  /* Iterate over active and visible sprites and blit them */
  /* layer & 0xc000U == 0 */
  if ( ( sptptr->sprite.layer & RL_SPRITE_FLAGS_MASK ) == 0 )
  {
    do
    {
      sptptr->bg = saved_ptr;
      saved_ptr = rl_image_blit( sptptr->sprite.image, sptptr->sprite.x, sptptr->sprite.y, saved_ptr );
      sptptr++;
    }
    while ( ( sptptr->sprite.layer & RL_SPRITE_FLAGS_MASK ) == 0 );
  }
  
  num_visible = sptptr - sprites;
  
  /* Jump over active but invisible sprites */
  /* layer & 0xc000U == 0x4000U */
  if ( ( sptptr->sprite.layer & RL_SPRITE_FLAGS_MASK ) == RL_SPRITE_INVISIBLE )
  {
    do
    {
      sptptr++;
    }
    while ( ( sptptr->sprite.layer & RL_SPRITE_FLAGS_MASK ) == RL_SPRITE_INVISIBLE );
  }
  
  num_sprites = sptptr - sprites;
}

void rl_sprites_end( void )
{
  spt_t* sptptr = sprites + num_visible - 1;
  
  /* Unblit the sprites in reverse order */
  if ( sptptr >= sprites )
  {
    do
    {
      rl_image_unblit( sptptr->sprite.image, sptptr->sprite.x, sptptr->sprite.y, sptptr->bg );
      sptptr--;
    }
    while ( sptptr >= sprites );
  }
}
