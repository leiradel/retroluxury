#include <libretro.h>

#include <rl_backgrnd.h>
#include <rl_bdffont.h>
#include <rl_config.h>
#include <rl_image.h>
#include <rl_pack.h>
#include <rl_pixelsrc.h>
#include <rl_rand.h>
#include <rl_sound.h>
#include <rl_sprite.h>

#include <stdlib.h>
#include <time.h>

#define WIDTH  320
#define HEIGHT 240
#define COUNT  8

typedef struct
{
  rl_sprite_t* sprite;
  int dx, dy;
}
smile_t;

typedef struct
{
  rl_pixelsrc_t pixelsrc;
  rl_image_t    image;
  smile_t       smiles[ COUNT ];
  rl_sound_t    music;
  rl_sound_t    sound;
  rl_bdffont_t  font;
  rl_image_t    text;
  rl_sprite_t*  text_spt;
  rl_rand_t     rand;
}
state_t;

static void dummy_log( enum retro_log_level level, const char* fmt, ... )
{
  (void)level;
  (void)fmt;
}

static retro_video_refresh_t      video_cb;
static retro_input_poll_t         input_poll_cb;
static retro_environment_t        env_cb;
static retro_log_printf_t         log_cb = dummy_log;
static retro_audio_sample_batch_t audio_cb;

static state_t state;

void retro_get_system_info( struct retro_system_info* info )
{
  info->library_name = "retroluxury test";
  info->library_version = "0.1";
  info->need_fullpath = true;
  info->block_extract = false;
  info->valid_extensions = "zip";
}

void retro_set_environment( retro_environment_t cb )
{
  env_cb = cb;

  bool yes = true;
  cb( RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME, &yes );
}

unsigned retro_api_version( void )
{
  return RETRO_API_VERSION;
}

void retro_init( void )
{
  struct retro_log_callback log;

  if ( env_cb( RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &log ) )
  {
    log_cb = log.log;
  }
}

bool retro_load_game( const struct retro_game_info* info )
{
  enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_RGB565;

  if ( !env_cb( RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt ) )
  {
    log_cb( RETRO_LOG_ERROR, "retroluxury needs RGB565\n" );
    return false;
  }

  rl_sound_init();
  rl_image_init();
  rl_sprite_init();
  rl_rand_create( &state.rand, time( NULL ) );

  rl_pack_init( NULL, "retroluxury", "libretro_demo" );
  rl_pack_add( "/home/leiradel/Develop/retroluxury/test/pack.zip" );

  rl_backgrnd_create( WIDTH, HEIGHT, RL_BACKGRND_EXACT );
  rl_backgrnd_clear( RL_COLOR( 64, 64, 64 ) );
  
  rl_pixelsrc_create( &state.pixelsrc, "/smile.png" );
  rl_image_create( &state.image, &state.pixelsrc, 0, 0 );

  for ( unsigned i = 0; i < COUNT; i++ )
  {
    rl_sprite_t* sprite = rl_sprite_create();

    sprite->x = rl_rand_interval( &state.rand, 0, WIDTH - state.pixelsrc.width );
    sprite->y = rl_rand_interval( &state.rand, 0, HEIGHT - state.pixelsrc.height );
    sprite->layer = i;
    sprite->image = &state.image;

    state.smiles[ i ].sprite = sprite;
    state.smiles[ i ].dx = rand() & 1 ? -1 : 1;
    state.smiles[ i ].dy = rand() & 1 ? -1 : 1;
  }

  rl_sound_stream( &state.music, "/sketch008.ogg" );
  rl_sound_sfxr( &state.sound, RL_SOUND_SFXR_COIN, 1 );
  rl_bdffont_create( &state.font, "/b10.bdf" );

  rl_pixelsrc_t pixelsrc;
  int x0, y0;
  rl_bdffont_render( &pixelsrc, &state.font, &x0, &y0, "retroluxury demo", 0, 0xffffffff );
  rl_image_create( &state.text, &pixelsrc, 0, 0 );
  rl_pixelsrc_destroy( &pixelsrc );
  state.text_spt = rl_sprite_create();
  state.text_spt->x = ( WIDTH - state.text.width ) / 2;
  state.text_spt->y = ( HEIGHT - state.text.height ) / 2;
  state.text_spt->layer = COUNT + 1;
  state.text_spt->image = &state.text;
  
  rl_sound_play( &state.music, 0.2f, 1 );
  return true;
}

size_t retro_get_memory_size( unsigned id )
{
  return 0;
}

void* retro_get_memory_data( unsigned id )
{
  return NULL;
}

void retro_set_video_refresh( retro_video_refresh_t cb )
{
  video_cb = cb;
}

void retro_set_audio_sample( retro_audio_sample_t cb )
{
  (void)cb;
}

void retro_set_audio_sample_batch( retro_audio_sample_batch_t cb )
{
  audio_cb = cb;
}

void retro_set_input_state( retro_input_state_t cb )
{
  (void)cb;
}

void retro_set_input_poll( retro_input_poll_t cb )
{
  input_poll_cb = cb;
}

void retro_get_system_av_info( struct retro_system_av_info* info )
{
  const rl_config_t* config = rl_get_config();

  info->geometry.base_width = WIDTH;
  info->geometry.base_height = HEIGHT;
  info->geometry.max_width = WIDTH + config->backgrnd_margin;
  info->geometry.max_height = HEIGHT;
  info->geometry.aspect_ratio = 0.0f;
  info->timing.fps = config->frame_rate;
  info->timing.sample_rate = config->sample_rate;
}

void retro_run( void )
{
  static bool first_time = true;

  if ( first_time )
  {
    first_time = false;
  }
  else
  {
    rl_sprites_unblit();
  }

  const rl_config_t* config = rl_get_config();

  input_poll_cb();

  unsigned width = WIDTH - state.pixelsrc.width;
  unsigned height = HEIGHT - state.pixelsrc.height;

  for ( unsigned i = 0; i < COUNT; i++ )
  {
    int bounced = 0;

    smile_t* smile = state.smiles + i;

    smile->sprite->x += smile->dx;
    smile->sprite->y += smile->dy;

    if ( smile->sprite->x < 0 || smile->sprite->x > width )
    {
      smile->dx = -smile->dx;
      smile->sprite->x += smile->dx;
      bounced = 1;
    }

    if ( smile->sprite->y < 0 || smile->sprite->y > height )
    {
      smile->dy = -smile->dy;
      smile->sprite->y += smile->dy;
      bounced = 1;
    }

    if ( bounced )
    {
      rl_sound_play( &state.sound, 1.0f, 0 );
    }
  }

  rl_sprites_blit();
  video_cb( (void*)rl_backgrnd_fb( NULL, NULL ), WIDTH, HEIGHT, ( WIDTH + config->backgrnd_margin ) * 2 );

  audio_cb( rl_sound_mix(), config->samples_per_frame );
}

void retro_deinit( void )
{
}

void retro_set_controller_port_device( unsigned port, unsigned device )
{
  (void)port;
  (void)device;
}

void retro_reset( void )
{
}

size_t retro_serialize_size( void )
{
  log_cb( RETRO_LOG_INFO, "Returning 4k byte state size\n" );
  return 4096;
}

bool retro_serialize( void* data, size_t size )
{
  log_cb( RETRO_LOG_INFO, "Sarializing %zu bytes to %p\n", size, data );
  return true;
}

bool retro_unserialize( const void* data, size_t size )
{
  log_cb( RETRO_LOG_INFO, "Unserializing %zu bytes from %p\n", size, data );
  return true;
}

void retro_cheat_reset( void )
{
}

void retro_cheat_set( unsigned a, bool b, const char* c )
{
  (void)a;
  (void)b;
  (void)c;
}

bool retro_load_game_special( unsigned a, const struct retro_game_info* b, size_t c )
{
  (void)a;
  (void)b;
  (void)c;
  return false;
}

void retro_unload_game( void )
{
  rl_sprite_destroy( state.text_spt );
  rl_image_destroy( &state.text );
  rl_sound_destroy( &state.sound );
  rl_sound_destroy( &state.music );

  for ( unsigned i = 0; i < COUNT; i++ )
  {
    rl_sprite_destroy( state.smiles[ i ].sprite );
  }

  rl_pixelsrc_destroy( &state.pixelsrc );
  rl_image_destroy( &state.image );
  rl_backgrnd_destroy();

  rl_pack_done();
  rl_sound_done();
}

unsigned retro_get_region( void )
{
  return RETRO_REGION_PAL;
}
