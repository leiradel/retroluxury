#include <libretro.h>

#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <stdarg.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <rl_backgrnd.h>
#include <rl_sound.h>

#include <boot_lua.h>

extern const char* rl_gitstamp;

void* constcast( const void* ptr );
void  register_rl( lua_State* L, void* pack, retro_input_state_t* input_state_cb, retro_video_refresh_t* video_cb );

/*---------------------------------------------------------------------------*/

static void dummy_log( enum retro_log_level level, const char* fmt, ... )
{
  (void)level;
  (void)fmt;
}

static retro_log_printf_t         log_cb = dummy_log;
static retro_environment_t        env_cb;
static retro_video_refresh_t      video_cb;
static retro_audio_sample_batch_t audio_cb;
static retro_input_poll_t         input_poll_cb;
static retro_input_state_t        input_state_cb;
static struct retro_perf_callback perf_cb;

/*---------------------------------------------------------------------------*/

#define MAX_PADS 4
static unsigned input_devices[ MAX_PADS ];

static struct retro_input_descriptor input_descriptors[] =
{
  { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP, "Up" },
  { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN, "Down" },
  { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT, "Left" },
  { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "Right" },
  { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A, "A" },
  { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B, "B" },
  { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X, "X" },
  { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y, "Y" },
  { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L, "L1" },
  { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R, "R1" },
  { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L2, "L2" },
  { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R2, "R2" },
  { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L3, "L3" },
  { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R3, "R3" },
  { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT, "Select" },
  { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START, "Start" },
  // TODO: Is this needed?
  { 255, 255, 255, 255, "" }
};

static bool input_state[ MAX_PADS ][ sizeof( input_descriptors ) / sizeof( input_descriptors[ 0 ] ) - 1 ];
 
/*---------------------------------------------------------------------------*/

#ifdef LOG_PERFORMANCE
#define RETRO_PERFORMANCE_INIT( name )  static struct retro_perf_counter name = {#name}; if ( !name.registered ) perf_cb.perf_register( &( name ) )
#define RETRO_PERFORMANCE_START( name ) perf_cb.perf_start( & ( name ) )
#define RETRO_PERFORMANCE_STOP( name )  perf_cb.perf_stop( & ( name ) )
#else
#define RETRO_PERFORMANCE_INIT( name )
#define RETRO_PERFORMANCE_START( name )
#define RETRO_PERFORMANCE_STOP( name )
#endif

/*---------------------------------------------------------------------------*/

typedef struct
{
  lua_State* L;
  int        tick_ref;
}
corestate_t;

static corestate_t state;

/*---------------------------------------------------------------------------*/

static int l_init_state( lua_State* L )
{
#ifndef NDEBUG
  lua_pushboolean( L, 1 );
  lua_setglobal( L, "_DEBUG" );
#endif
  
  /* Register rl functions and types. */
  void* pack = lua_touserdata( L, 1 );
  register_rl( L, pack, &input_state_cb, &video_cb );
  
  /* Execute boot.lua. */
  if ( luaL_loadbufferx( L, boot_lua, boot_lua_len, "boot.lua", "t" ) != LUA_OK )
  {
    return lua_error( L );
  }
  
  lua_call( L, 0, 1 );
  
  /* Make a reference to the tick function. */
  state.tick_ref = luaL_ref( L, LUA_REGISTRYINDEX );
}

static int l_traceback( lua_State* L )
{
  luaL_traceback( L, L, lua_tostring( L, -1 ), 1 );
  return 1;
}

static int l_pcall( lua_State* L, int nargs, int nres )
{
  lua_pushcfunction( L, l_traceback );
  lua_insert( L, -nargs - 2 );
  
  if ( lua_pcall( L, nargs, nres, -nargs - 2 ) != LUA_OK )
  {
    log_cb( RETRO_LOG_ERROR, "\n==============================\n%s\n------------------------------\n", lua_tostring( L, -1 ) );
    return -1;
  }
  
  return 0;
}

static int init_state( void* pack )
{
  state.L = luaL_newstate();
  
  if ( !state.L )
  {
    log_cb( RETRO_LOG_ERROR, "Error creating Lua state" );
    return -1;
  }
  
  luaL_openlibs( state.L );
  
  lua_pushcfunction( state.L, l_init_state );
  lua_pushlightuserdata( state.L, pack );
  
  return l_pcall( state.L, 1, 0 );
}

/*---------------------------------------------------------------------------*/

void retro_get_system_info( struct retro_system_info* info )
{
  info->library_name = "Retro Luxury";
  info->library_version = "1.0";
  info->need_fullpath = false;
  info->block_extract = false;
  info->valid_extensions = "rlp";
}

void retro_set_environment( retro_environment_t cb )
{
  env_cb = cb;
  
  static const struct retro_variable vars[] = {
    { NULL, NULL },
  };
  
  static const struct retro_controller_description controllers[] = {
    { "Controller", RETRO_DEVICE_JOYPAD },
    { NULL, 0 }
  };
  
  static const struct retro_controller_info ports[ MAX_PADS + 1 ] = {
    { controllers, sizeof( controllers ) / sizeof( controllers [ 0 ] ) },
    { controllers, sizeof( controllers ) / sizeof( controllers [ 0 ] ) },
    { controllers, sizeof( controllers ) / sizeof( controllers [ 0 ] ) },
    { controllers, sizeof( controllers ) / sizeof( controllers [ 0 ] ) },
    { NULL, 0 }
  };
  
  cb( RETRO_ENVIRONMENT_SET_VARIABLES, (void*)vars );
  cb( RETRO_ENVIRONMENT_SET_CONTROLLER_INFO, (void*)ports );
}

unsigned retro_api_version()
{
  return RETRO_API_VERSION;
}

void retro_init()
{
  struct retro_log_callback log;
  
  if ( env_cb( RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &log ) )
  {
    log_cb = log.log;
  }
  
  if ( !env_cb( RETRO_ENVIRONMENT_GET_PERF_INTERFACE, &perf_cb ) )
  {
    perf_cb.get_time_usec = NULL;
    log_cb( RETRO_LOG_WARN, "Could not get the perf interface\n" );
  }
  
  log_cb( RETRO_LOG_ERROR, "\n%s", rl_gitstamp );
}

bool retro_load_game( const struct retro_game_info* info )
{
  if ( !perf_cb.get_time_usec )
  {
    log_cb( RETRO_LOG_ERROR, "Core needs the perf interface\n" );
    return false;
  }
  
  enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_RGB565;
  
  if ( !env_cb( RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt ) )
  {
    log_cb( RETRO_LOG_ERROR, "RGB565 is not supported\n" );
    return false;
  }
  
  env_cb( RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, input_descriptors );
  memset( input_state, 0, sizeof( input_state ) );
  
  return !init_state( constcast( info->data ) );
}

size_t retro_get_memory_size( unsigned id )
{
  (void)id;
  return 0;
}

void* retro_get_memory_data( unsigned id )
{
  (void)id;
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
  input_state_cb = cb;
}

void retro_set_input_poll( retro_input_poll_t cb )
{
  input_poll_cb = cb;
}

void retro_get_system_av_info( struct retro_system_av_info* info )
{
  int width, height;
  rl_backgrnd_fb( &width, &height );
  
  info->geometry.base_width   = width;
  info->geometry.base_height  = height;
  info->geometry.max_width    = width;
  info->geometry.max_height   = height;
  info->geometry.aspect_ratio = 0.0f;
  info->timing.fps            = 60.0;
  info->timing.sample_rate    = 44100.0;
}

void retro_run()
{
  input_poll_cb();
  
  lua_rawgeti( state.L, LUA_REGISTRYINDEX, state.tick_ref );
  l_pcall( state.L, 0, 0 );
  
  audio_cb( rl_sound_mix(), RL_SAMPLES_PER_FRAME );
}

void retro_deinit()
{
#ifdef LOG_PERFORMANCE
  perf_cb.perf_log();
#endif
}

void retro_set_controller_port_device( unsigned port, unsigned device )
{
  switch ( device )
  {
  default:
    device = RETRO_DEVICE_JOYPAD;
    // fallthrough
    
  case RETRO_DEVICE_JOYPAD:
    input_devices[ port ] = device;
    break;
  }
}

void retro_reset()
{
}

size_t retro_serialize_size()
{
  return 0;
}

bool retro_serialize( void* data, size_t size )
{
  (void)data;
  (void)size;
  return false;
}

bool retro_unserialize( const void* data, size_t size )
{
  (void)data;
  (void)size;
  return false;
}

void retro_cheat_reset()
{
}

void retro_cheat_set( unsigned a, bool b, const char* c )
{
  (void)a;
  (void)b;
  (void)c;
}

bool retro_load_game_special(unsigned a, const struct retro_game_info* b, size_t c)
{
  (void)a;
  (void)b;
  (void)c;
  return false;
}

void retro_unload_game()
{
  lua_close( state.L );
}

unsigned retro_get_region()
{
  return RETRO_REGION_NTSC;
}
