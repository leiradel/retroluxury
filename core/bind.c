#include <lua.h>
#include <lauxlib.h>

#include <rl_pack.h>
#include <rl_sound.h>
#include <rl_backgrnd.h>
#include <rl_rand.h>
#include <rl_image.h>
#include <rl_memory.h>
#include <rl_tile.h>
#include <rl_sprite.h>
#include <rl_map.h>

#include <time.h>

#include <libretro.h>

static inline size_t find_entry( lua_State* L, int index, const char* type, void** data )
{
  void*       pack  = lua_touserdata( L, lua_upvalueindex( 1 ) );
  const char* name  = luaL_checkstring( L, index );
  rl_entry_t* entry = rl_find_entry( pack, name );
  
  if ( entry )
  {
    *data = (char*)pack + entry->data_offset;
    return entry->data_size;
  }
  
  return luaL_error( L, "%s \"%s\" not found", type, name );
}

/*****************************************************************************/

#define sound_ref   ud[ 1 ].i
#define stop_cb_ref ud[ 2 ].i
#define auto_ref    ud[ 3 ].i

static void sound_stop_cb( rl_voice_t* voice, int reason )
{
  lua_State* L = (lua_State*)voice->ud[ 0 ].p;
  
  if ( voice->stop_cb_ref != LUA_NOREF )
  {
    lua_rawgeti( L, LUA_REGISTRYINDEX, voice->stop_cb_ref );
    
    if ( voice->sound_ref != LUA_NOREF )
    {
      lua_rawgeti( L, LUA_REGISTRYINDEX, voice->sound_ref );
    }
    else
    {
      lua_pushnil( L );
    }
    
    switch ( reason )
    {
    case RL_SOUND_FINISHED: lua_pushliteral( L, "finished" ); break;
    case RL_SOUND_STOPPED:  lua_pushliteral( L, "stopped" );  break;
    case RL_SOUND_REPEATED: lua_pushliteral( L, "repeated" ); break;
    
    default:
      luaL_error( L, "unknown reason in stop callback: %d", reason );
      return;
    }
    
    lua_call( L, 2, 0 );
    luaL_unref( L, LUA_REGISTRYINDEX, voice->stop_cb_ref );
  }
  
  if ( voice->sound_ref != LUA_NOREF )
  {
    luaL_unref( L, LUA_REGISTRYINDEX, voice->sound_ref );
  }
  
  luaL_unref( L, LUA_REGISTRYINDEX, voice->auto_ref );
}

static int l_voice_stop( lua_State* L )
{
  rl_voice_t* self = *(rl_voice_t**)luaL_checkudata( L, 1, "voice" );
  rl_sound_stop( self );
  return 0;
} 

static int push_voice( lua_State* L, rl_voice_t* voice )
{
  static const luaL_Reg methods[] =
  {
    { "stop", l_voice_stop },
    { NULL, NULL }
  };
  
  rl_voice_t** ud = (rl_voice_t**)lua_newuserdata( L, sizeof( rl_voice_t* ) );
  *ud = voice;
  
  lua_pushvalue( L, -1 );
  voice->auto_ref = luaL_ref( L, LUA_REGISTRYINDEX );
  voice->ud[ 0 ].p = (void*)L;
  
  if ( luaL_newmetatable( L, "voice" ) != 0 )
  {
    lua_pushvalue( L, -1 );
    lua_setfield( L, -2, "__index" );
    luaL_setfuncs( L, methods, 0 );
  }
  
  lua_setmetatable( L, -2 );
  return 1;
}

/*****************************************************************************/

static int l_loadFile( lua_State* L )
{
  void*  data;
  size_t size = find_entry( L, 1, "file", &data );
  
  lua_pushlstring( L, (char*)data, size );
  return 1;
}

static int l_create( lua_State* L )
{
  int width  = luaL_checkinteger( L, 1 );
  int height = luaL_checkinteger( L, 2 );
  
  if ( width <= 0 || height <= 0 )
  {
    return luaL_error( L, "invalid value for width and/or height" );
  }
  
  if ( rl_backgrnd_create( width, height, RL_BACKGRND_EXACT ) )
  {
    return luaL_error( L, "error creating the background" );
  }
  
  rl_backgrnd_fb( &width, &height );
  
  lua_pushinteger( L, width );
  lua_pushinteger( L, height );
  return 2;
}

static int l_clear( lua_State* L )
{
  uint16_t color = luaL_checkinteger( L, 1 );
  rl_backgrnd_clear( color );
  return 0;
}

static int l_color( lua_State* L )
{
  int r = luaL_checkinteger( L, 1 ) * 32 / 256;
  int g = luaL_checkinteger( L, 2 ) * 64 / 256;
  int b = luaL_checkinteger( L, 3 ) * 32 / 256;
  
  lua_pushinteger( L, ( r << 11 ) | ( g << 5 ) | b );
  return 1;
}

static int l_randomseed( lua_State* L )
{
  rl_srand( luaL_checkinteger( L, 1 ) );
  return 0;
}

static int l_randomize( lua_State* L )
{
  rl_srand( time( NULL ) );
  return 0;
}

static int l_random( lua_State* L )
{
  switch ( lua_gettop( L ) )
  {
  case 0:
    {
      double frac = rl_rand() / 4294967296.0;
      lua_pushnumber( L, frac );
      return 1;
    }
    
  case 1:
    {
      int max = luaL_checkinteger( L, 1 );
      lua_pushinteger( L, rl_random( 1, max ) );
      return 1;
    }
    
  default:
    {
      int min = luaL_checkinteger( L, 1 );
      int max = luaL_checkinteger( L, 2 );
      lua_pushinteger( L, rl_random( min, max ) );
      return 1;
    }
  }
}

static int l_stopSounds( lua_State* L )
{
  rl_sound_stop_all();
  return 0;
}

static int l_playMusic( lua_State* L )
{
  void*  data;
  size_t size = find_entry( L, 1, "music", &data );
  
  int repeat = lua_toboolean( L, 2 );
  
  rl_voice_t* voice = rl_sound_play_ogg( data, size, repeat, sound_stop_cb );
  
  if ( voice )
  {
    voice->sound_ref = LUA_NOREF;
    
    if ( lua_isnoneornil( L, 3 ) )
    {
      voice->stop_cb_ref = LUA_NOREF;
    }
    else
    {
      luaL_checktype( L, 3, LUA_TFUNCTION );
      lua_pushvalue( L, 3 );
      voice->stop_cb_ref = luaL_ref( L, LUA_REGISTRYINDEX );
    }
    
    return push_voice( L, voice );
  }
  
  return luaL_error( L, "music already playing" );
}

static int l_getInputState( lua_State* L )
{
  static const struct { unsigned id; const char* name; } buttons[] = {
    { RETRO_DEVICE_ID_JOYPAD_UP,     "up" },
    { RETRO_DEVICE_ID_JOYPAD_DOWN,   "down" },
    { RETRO_DEVICE_ID_JOYPAD_LEFT,   "left" },
    { RETRO_DEVICE_ID_JOYPAD_RIGHT,  "right" },
    { RETRO_DEVICE_ID_JOYPAD_A,      "a" },
    { RETRO_DEVICE_ID_JOYPAD_B,      "b" },
    { RETRO_DEVICE_ID_JOYPAD_X,      "x" },
    { RETRO_DEVICE_ID_JOYPAD_Y,      "y" },
    { RETRO_DEVICE_ID_JOYPAD_L,      "l1" },
    { RETRO_DEVICE_ID_JOYPAD_R,      "r1" },
    { RETRO_DEVICE_ID_JOYPAD_L2,     "l2" },
    { RETRO_DEVICE_ID_JOYPAD_R2,     "r2" },
    { RETRO_DEVICE_ID_JOYPAD_L3,     "l3" },
    { RETRO_DEVICE_ID_JOYPAD_R3,     "r3" },
    { RETRO_DEVICE_ID_JOYPAD_SELECT, "select" },
    { RETRO_DEVICE_ID_JOYPAD_START,  "start" },
  };
  
  retro_input_state_t input_state_cb = *(retro_input_state_t*)lua_touserdata( L, lua_upvalueindex( 2 ) );
  unsigned port = luaL_checkinteger( L, 1 ) - 1;
  
  if ( lua_type( L, 2 ) == LUA_TTABLE )
  {
    lua_pushvalue( L, 2 );
  }
  else
  {
    lua_createtable( L, 0, sizeof( buttons ) / sizeof( buttons[ 0 ] ) );
  }
  
  for ( int i = 0; i < sizeof( buttons ) / sizeof( buttons[ 0 ] ); i++ )
  {
    int16_t pressed = input_state_cb( port, RETRO_DEVICE_JOYPAD, 0, buttons[ i ].id );
    lua_pushboolean( L, pressed );
    lua_setfield( L, -2, buttons[ i ].name );
  }
  
  return 1;
}

static int l_presentVideo( lua_State* L )
{
  retro_video_refresh_t video_cb = *(retro_video_refresh_t*)lua_touserdata( L, lua_upvalueindex( 3 ) );
  
  int width, height;
  uint16_t* fb = rl_backgrnd_fb( &width, &height );
  
  video_cb( fb, width, height, ( width + RL_BACKGRND_MARGIN ) * sizeof( uint16_t ) );
  return 0;
}

/*****************************************************************************/

static int l_image_getWidth( lua_State* L )
{
  rl_image_t* self = *(rl_image_t**)luaL_checkudata( L, 1, "image" );
  lua_pushinteger( L, self->width );
  return 1;
}

static int l_image_getHeight( lua_State* L )
{
  rl_image_t* self = *(rl_image_t**)luaL_checkudata( L, 1, "image" );
  lua_pushinteger( L, self->height );
  return 1;
}

static int l_image_getSize( lua_State* L )
{
  rl_image_t* self = *(rl_image_t**)luaL_checkudata( L, 1, "image" );
  lua_pushinteger( L, self->width );
  lua_pushinteger( L, self->height );
  return 2;
}

static int l_image_blit( lua_State* L )
{
  rl_image_t* self = *(rl_image_t**)luaL_checkudata( L, 1, "image" );
  int x = luaL_checkinteger( L, 2 );
  int y = luaL_checkinteger( L, 3 );
  
  rl_image_blit_nobg( self, x, y );
  return 0;
}

static int l_image_gc( lua_State* L )
{
  rl_image_t* self = *(rl_image_t**)luaL_checkudata( L, 1, "image" );
  
  if ( self->ud[ 0 ].i != LUA_NOREF )
  {
    /* Image is from an image set. */
    if ( --self->ud[ 1 ].i == 0 )
    {
      luaL_unref( L, LUA_REGISTRYINDEX, self->ud[ 0 ].i );
      self->ud[ 0 ].i = LUA_NOREF;
    }
  }
  else
  {
    /* Stand alone image. */
    rl_image_destroy( self );
  }
  
  return 0;
}

static int push_image( lua_State* L, rl_image_t* image )
{
  static const luaL_Reg methods[] =
  {
    { "getWidth",  l_image_getWidth },
    { "getHeight", l_image_getHeight },
    { "getSize",   l_image_getSize },
    { "blit",      l_image_blit },
    { "__gc",      l_image_gc },
    { NULL, NULL }
  };
  
  rl_image_t** ud = (rl_image_t**)lua_newuserdata( L, sizeof( rl_image_t* ) );
  *ud = image;
  
  if ( luaL_newmetatable( L, "image" ) != 0 )
  {
    lua_pushvalue( L, -1 );
    lua_setfield( L, -2, "__index" );
    luaL_setfuncs( L, methods, 0 );
  }
  
  lua_setmetatable( L, -2 );
  return 1;
}

static int l_loadImage( lua_State* L )
{
  void*  data;
  size_t size = find_entry( L, 1, "image", &data );
  
  rl_image_t* self = rl_image_create( data, size );
  
  if ( self )
  {
    self->ud[ 0 ].i = LUA_NOREF;
    return push_image( L, self );
  }
  
  return luaL_error( L, "error loading image" );
}

/*****************************************************************************/

static int l_imageset_getWidth( lua_State* L )
{
  rl_imageset_t* self = *(rl_imageset_t**)luaL_checkudata( L, 1, "imageset" );
  int index = luaL_checkinteger( L, 2 );
  
  if ( index >= 0 && index < self->num_images )
  {
    lua_pushinteger( L, self->images[ index ]->width );
    return 1;
  }
  
  return luaL_error( L, "image index out of range" );
}

static int l_imageset_getHeight( lua_State* L )
{
  rl_imageset_t* self = *(rl_imageset_t**)luaL_checkudata( L, 1, "imageset" );
  int index = luaL_checkinteger( L, 2 );
  
  if ( index >= 0 && index < self->num_images )
  {
    lua_pushinteger( L, self->images[ index ]->height );
    return 1;
  }
  
  return luaL_error( L, "image index out of range" );
}

static int l_imageset_getSize( lua_State* L )
{
  rl_imageset_t* self = *(rl_imageset_t**)luaL_checkudata( L, 1, "imageset" );
  int index = luaL_checkinteger( L, 2 );
  
  if ( index >= 0 && index < self->num_images )
  {
    lua_pushinteger( L, self->images[ index ]->width );
    lua_pushinteger( L, self->images[ index ]->height );
    return 2;
  }
  
  return luaL_error( L, "image index out of range" );
}

static int l_imageset_getNumImages( lua_State* L )
{
  rl_imageset_t* self = *(rl_imageset_t**)luaL_checkudata( L, 1, "imageset" );
  lua_pushinteger( L, self->num_images );
  return 1;
}

static int l_imageset_blit( lua_State* L )
{
  rl_imageset_t* self = *(rl_imageset_t**)luaL_checkudata( L, 1, "imageset" );
  int index = luaL_checkinteger( L, 2 );
  
  if ( index >= 0 && index < self->num_images )
  {
    int x = luaL_checkinteger( L, 3 );
    int y = luaL_checkinteger( L, 4 );
    
    rl_image_blit_nobg( self->images[ index ], x, y );
    return 0;
  }
  
  return luaL_error( L, "image index out of range" );
}

static int l_imageset_getImage( lua_State* L )
{
  rl_imageset_t* self = *(rl_imageset_t**)luaL_checkudata( L, 1, "imageset" );
  int index = luaL_checkinteger( L, 2 );
  
  if ( index >= 0 && index < self->num_images )
  {
    if ( self->images[ index ]->ud[ 0 ].i == LUA_NOREF )
    {
      /* First time pushed, create a reference to the imageset. */
      lua_pushvalue( L, 1 );
      self->images[ index ]->ud[ 0 ].i = luaL_ref( L, LUA_REGISTRYINDEX );
      /* Set the reference count. */
      self->images[ index ]->ud[ 1 ].i = 1;
    }
    else
    {
      /* Increment the reference count. */
      self->images[ index ]->ud[ 1 ].i++;
    }
    
    return push_image( L, self->images[ index ] );
  }
  
  return luaL_error( L, "image index out of range" );
}

static int l_imageset_gc( lua_State* L )
{
  rl_imageset_t* self = *(rl_imageset_t**)luaL_checkudata( L, 1, "imageset" );
  rl_imageset_destroy( self );
  return 0;
}

static int l_loadImageSet( lua_State* L )
{
  static const luaL_Reg methods[] =
  {
    { "getWidth",     l_imageset_getWidth },
    { "getHeight",    l_imageset_getHeight },
    { "getSize",      l_imageset_getSize },
    { "getNumImages", l_imageset_getNumImages },
    { "blit",         l_imageset_blit },
    { "getImage",     l_imageset_getImage },
    { "__gc",         l_imageset_gc },
    { NULL, NULL }
  };
  
  void*  data;
  size_t size = find_entry( L, 1, "imageset", &data );
  
  rl_imageset_t* self = rl_imageset_create( data, size );
  
  if ( self )
  {
    rl_imageset_t** ud = (rl_imageset_t**)lua_newuserdata( L, sizeof( rl_imageset_t* ) );
    *ud = self;
    
    for ( int i = 0; i < self->num_images; i++ )
    {
      self->images[ i ]->ud[ 0 ].i = LUA_NOREF;
    }
    
    if ( luaL_newmetatable( L, "imageset" ) != 0 )
    {
      lua_pushvalue( L, -1 );
      lua_setfield( L, -2, "__index" );
      luaL_setfuncs( L, methods, 0 );
    }
    
    lua_setmetatable( L, -2 );
    return 1;
  }
  
  return luaL_error( L, "error loading imageset" );
}

/*****************************************************************************/

static int l_tileset_getWidth( lua_State* L )
{
  rl_tileset_t* self = *(rl_tileset_t**)luaL_checkudata( L, 1, "tileset" );
  lua_pushinteger( L, self->width );
  return 1;
}

static int l_tileset_getHeight( lua_State* L )
{
  rl_tileset_t* self = *(rl_tileset_t**)luaL_checkudata( L, 1, "tileset" );
  lua_pushinteger( L, self->height );
  return 1;
}

static int l_tileset_getSize( lua_State* L )
{
  rl_tileset_t* self = *(rl_tileset_t**)luaL_checkudata( L, 1, "tileset" );
  lua_pushinteger( L, self->width );
  lua_pushinteger( L, self->height );
  return 2;
}

static int l_tileset_getNumTiles( lua_State* L )
{
  rl_tileset_t* self = *(rl_tileset_t**)luaL_checkudata( L, 1, "tileset" );
  lua_pushinteger( L, self->num_tiles );
  return 1;
}

static int l_tileset_blit( lua_State* L )
{
  rl_tileset_t* self = *(rl_tileset_t**)luaL_checkudata( L, 1, "tileset" );
  int index = luaL_checkinteger( L, 2 );
  
  if ( index >= 0 && index < self->num_tiles )
  {
    int x = luaL_checkinteger( L, 3 );
    int y = luaL_checkinteger( L, 4 );
    
    rl_tileset_blit_nobg( self, index, x, y );
    return 0;
  }
  
  return luaL_error( L, "tile index out of range" );
}

static int l_tileset_gc( lua_State* L )
{
  rl_tileset_t* self = *(rl_tileset_t**)luaL_checkudata( L, 1, "tileset" );
  rl_tileset_destroy( self );
  return 0;
}

static int l_loadTileSet( lua_State* L )
{
  static const luaL_Reg methods[] =
  {
    { "getWidth",    l_tileset_getWidth },
    { "getHeight",   l_tileset_getHeight },
    { "getSize",     l_tileset_getSize },
    { "getNumTiles", l_tileset_getNumTiles },
    { "blit",        l_tileset_blit },
    { "__gc",        l_tileset_gc },
    { NULL, NULL }
  };
  
  void*  data;
  size_t size = find_entry( L, 1, "tileset", &data );
  
  rl_tileset_t* self = rl_tileset_create( data, size );
  
  if ( self )
  {
    rl_tileset_t** ud = (rl_tileset_t**)lua_newuserdata( L, sizeof( rl_tileset_t* ) );
    *ud = self;
    
    if ( luaL_newmetatable( L, "tileset" ) != 0 )
    {
      lua_pushvalue( L, -1 );
      lua_setfield( L, -2, "__index" );
      luaL_setfuncs( L, methods, 0 );
    }
    
    lua_setmetatable( L, -2 );
    return 1;
  }
  
  return luaL_error( L, "error loading tileset" );
}

/*****************************************************************************/

static int l_sprite_setLayer( lua_State* L )
{
  rl_sprite_t* self = *(rl_sprite_t**)luaL_checkudata( L, 1, "sprite" );
  self->layer = luaL_checkinteger( L, 2 );
  return 0;
}

static int l_sprite_getLayer( lua_State* L )
{
  rl_sprite_t* self = *(rl_sprite_t**)luaL_checkudata( L, 1, "sprite" );
  lua_pushinteger( L, self->layer );
  return 1;
}

static int l_sprite_setVisible( lua_State* L )
{
  rl_sprite_t* self = *(rl_sprite_t**)luaL_checkudata( L, 1, "sprite" );
  
  if ( lua_toboolean( L, 2 ) )
  {
    self->flags &= ~RL_SPRITE_INVISIBLE;
  }
  else
  {
    self->flags |= RL_SPRITE_INVISIBLE;
  }
  
  return 0;
}

static int l_sprite_getVisible( lua_State* L )
{
  rl_sprite_t* self = *(rl_sprite_t**)luaL_checkudata( L, 1, "sprite" );
  lua_pushboolean( L, !( self->flags & RL_SPRITE_INVISIBLE ) );
  return 0;
}

static int l_sprite_setX( lua_State* L )
{
  rl_sprite_t* self = *(rl_sprite_t**)luaL_checkudata( L, 1, "sprite" );
  self->x = luaL_checkinteger( L, 2 );
  return 0;
}

static int l_sprite_setY( lua_State* L )
{
  rl_sprite_t* self = *(rl_sprite_t**)luaL_checkudata( L, 1, "sprite" );
  self->y = luaL_checkinteger( L, 2 );
  return 0;
}

static int l_sprite_setPosition( lua_State* L )
{
  rl_sprite_t* self = *(rl_sprite_t**)luaL_checkudata( L, 1, "sprite" );
  self->x = luaL_checkinteger( L, 2 );
  self->y = luaL_checkinteger( L, 3 );
  return 0;
}

static int l_sprite_setImage( lua_State* L )
{
  rl_sprite_t* self = *(rl_sprite_t**)luaL_checkudata( L, 1, "sprite" );
  
  if ( self->ud[ 0 ].i != LUA_NOREF )
  {
    self->image = NULL;
    
    luaL_unref( L, LUA_REGISTRYINDEX, self->ud[ 0 ].i );
    self->ud[ 0 ].i = LUA_NOREF;
  }
  
  if ( !lua_isnoneornil( L, 2 ) )
  {
    self->image = *(rl_image_t**)luaL_checkudata( L, 2, "image" );
    
    lua_pushvalue( L, 2 );
    self->ud[ 0 ].i = luaL_ref( L, LUA_REGISTRYINDEX );
  }
  
  return 0;
}

static int l_sprite_getImage( lua_State* L )
{
  rl_sprite_t* self = *(rl_sprite_t**)luaL_checkudata( L, 1, "sprite" );
  
  if ( self->ud[ 0 ].i != LUA_NOREF )
  {
    lua_rawgeti( L, LUA_REGISTRYINDEX, self->ud[ 0 ].i );
    return 1;
  }
  
  return 0;
}

static int l_sprite_gc( lua_State* L )
{
  rl_sprite_t* self = *(rl_sprite_t**)luaL_checkudata( L, 1, "sprite" );
  
  if ( self->ud[ 0 ].i != LUA_NOREF )
  {
    luaL_unref( L, LUA_REGISTRYINDEX, self->ud[ 0 ].i );
  }
  
  rl_sprite_destroy( self );
  return 0;
}

static int l_newSprite( lua_State* L )
{
  static const luaL_Reg methods[] =
  {
    { "setLayer",    l_sprite_setLayer },
    { "getLayer",    l_sprite_getLayer },
    { "setVisible",  l_sprite_setVisible },
    { "getVisible",  l_sprite_getVisible },
    { "setX",        l_sprite_setX },
    { "setY",        l_sprite_setY },
    { "setPosition", l_sprite_setPosition },
    { "setImage",    l_sprite_setImage },
    { "getImage",    l_sprite_getImage },
    { "__gc",        l_sprite_gc },
    { NULL, NULL }
  };
  
  rl_sprite_t* self = rl_sprite_create();
  
  if ( self )
  {
    rl_sprite_t** ud = (rl_sprite_t**)lua_newuserdata( L, sizeof( rl_sprite_t* ) );
    *ud = self;
    
    self->ud[ 0 ].i = LUA_NOREF;
    
    if ( luaL_newmetatable( L, "sprite" ) != 0 )
    {
      lua_pushvalue( L, -1 );
      lua_setfield( L, -2, "__index" );
      luaL_setfuncs( L, methods, 0 );
    }
    
    lua_setmetatable( L, -2 );
    return 1;
  }
  
  return luaL_error( L, "error creating sprite" );
}

/*****************************************************************************/

static int l_sound_play( lua_State* L )
{
  rl_sound_t* self = *(rl_sound_t**)luaL_checkudata( L, 1, "sound" );
  int repeat = lua_toboolean( L, 2 );
  
  rl_voice_t* voice = rl_sound_play( self, repeat, sound_stop_cb );
  
  voice->ud[ 0 ].p = (void*)L;
  
  lua_pushvalue( L, 1 );
  voice->sound_ref = luaL_ref( L, LUA_REGISTRYINDEX );
  
  if ( lua_isnoneornil( L, 3 ) )
  {
    voice->stop_cb_ref = LUA_NOREF;
  }
  else
  {
    luaL_checktype( L, 3, LUA_TFUNCTION );
    lua_pushvalue( L, 3 );
    voice->stop_cb_ref = luaL_ref( L, LUA_REGISTRYINDEX );
  }
  
  return push_voice( L, voice );
}

static int l_sound_fire( lua_State* L )
{
  rl_sound_t* self = *(rl_sound_t**)luaL_checkudata( L, 1, "sound" );
  int repeat = lua_toboolean( L, 2 );
  
  rl_sound_play( self, repeat, NULL );
  return 0;
}

static int l_sound_gc( lua_State* L )
{
  rl_sound_t* self = *(rl_sound_t**)luaL_checkudata( L, 1, "sound" );
  rl_sound_destroy( self );
  return 0;
}

static int l_loadSound( lua_State* L )
{
  static const luaL_Reg methods[] =
  {
    { "play", l_sound_play },
    { "fire", l_sound_play },
    { "__gc", l_sound_gc },
    { NULL, NULL }
  };
  
  void*  data;
  size_t size   = find_entry( L, 1, "sound", &data );
  int    stereo = lua_toboolean( L, 2 );
  
  rl_sound_t* self = rl_sound_create( data, size, stereo );
  
  if ( self )
  {
    rl_sound_t** ud = (rl_sound_t**)lua_newuserdata( L, sizeof( rl_sound_t* ) );
    *ud = self;
    
    if ( luaL_newmetatable( L, "sound" ) != 0 )
    {
      lua_pushvalue( L, -1 );
      lua_setfield( L, -2, "__index" );
      luaL_setfuncs( L, methods, 0 );
    }
    
    lua_setmetatable( L, -2 );
    return 1;
  }
  
  return luaL_error( L, "error loading sound" );
}

/*****************************************************************************/

static int l_map_getWidth( lua_State* L )
{
  rl_map_t* self = *(rl_map_t**)luaL_checkudata( L, 1, "map" );
  lua_pushinteger( L, self->width );
  return 1;
}

static int l_map_getHeight( lua_State* L )
{
  rl_map_t* self = *(rl_map_t**)luaL_checkudata( L, 1, "map" );
  lua_pushinteger( L, self->height );
  return 1;
}

static int l_map_getSize( lua_State* L )
{
  rl_map_t* self = *(rl_map_t**)luaL_checkudata( L, 1, "map" );
  lua_pushinteger( L, self->width );
  lua_pushinteger( L, self->height );
  return 2;
}

static int l_map_getNumLayers( lua_State* L )
{
  rl_map_t* self = *(rl_map_t**)luaL_checkudata( L, 1, "map" );
  lua_pushinteger( L, self->num_layers );
  return 1;
}

static int l_map_hasCollision( lua_State* L )
{
  rl_map_t* self = *(rl_map_t**)luaL_checkudata( L, 1, "map" );
  lua_pushboolean( L, self->flags & RL_MAP_HAS_COLLISION );
  return 1;
}

static int l_map_getTileset( lua_State* L )
{
  rl_map_t* self = *(rl_map_t**)luaL_checkudata( L, 1, "map" );
  lua_rawgeti( L, LUA_REGISTRYINDEX, self->ud[ 0 ].i );
  return 1;
}

static int l_map_getImageset( lua_State* L )
{
  rl_map_t* self = *(rl_map_t**)luaL_checkudata( L, 1, "map" );
  lua_rawgeti( L, LUA_REGISTRYINDEX, self->ud[ 1 ].i );
  return 1;
}

static int l_map_isWalkable( lua_State* L )
{
  rl_map_t* self = *(rl_map_t**)luaL_checkudata( L, 1, "map" );
  int x = luaL_checkinteger( L, 2 );
  int y = luaL_checkinteger( L, 3 );
  
  if ( x >= 0 && x < self->width && y >= 0 && y < self->height )
  {
    int ndx = y * self->width + x;
    lua_pushboolean( L, self->collision[ ndx / 32 ] & ( 1 << ( ndx & 31 ) ) );
  }
  else
  {
    lua_pushboolean( L, 0 );
  }
  
  return 1;
}

static int l_map_blit0( lua_State* L )
{
  rl_map_t* self = *(rl_map_t**)luaL_checkudata( L, 1, "map" );
  int x = luaL_checkinteger( L, 2 );
  int y = luaL_checkinteger( L, 3 );
  
  rl_map_blit0_nobg( self, x, y );
  return 0;
}

static int l_map_blitn( lua_State* L )
{
  rl_map_t* self = *(rl_map_t**)luaL_checkudata( L, 1, "map" );
  int ndx = luaL_checkinteger( L, 2 );

  if ( ndx > 0 && ndx <= self->num_layers )
  {
    int x = luaL_checkinteger( L, 3 );
    int y = luaL_checkinteger( L, 4 );
    
    rl_map_blitn_nobg( self, ndx, x, y );
    return 0;
  }
  
  return luaL_error( L, "layer index out of range" );
}

static int l_map_gc( lua_State* L )
{
  rl_map_t* self = *(rl_map_t**)luaL_checkudata( L, 1, "map" );
  
  luaL_unref( L, LUA_REGISTRYINDEX, self->ud[ 0 ].i );
  luaL_unref( L, LUA_REGISTRYINDEX, self->ud[ 1 ].i );
  
  rl_map_destroy( self );
  return 0;
}

static int l_loadMap( lua_State* L )
{
  static const luaL_Reg methods[] =
  {
    { "getWidth",     l_map_getWidth },
    { "getHeight",    l_map_getHeight },
    { "getSize",      l_map_getSize },
    { "getNumLayers", l_map_getNumLayers },
    { "hasCollision", l_map_hasCollision },
    { "getTileset",   l_map_getTileset },
    { "getImageset",  l_map_getImageset },
    { "isWalkable",   l_map_isWalkable },
    { "blit0",        l_map_blit0 },
    { "blitn",        l_map_blitn },
    { "__gc",         l_map_gc },
    { NULL, NULL }
  };
  
  void*  data;
  size_t size = find_entry( L, 1, "map", &data );
  
  rl_tileset_t*  tileset  = *(rl_tileset_t**)luaL_checkudata( L, 2, "tileset" );
  rl_imageset_t* imageset = *(rl_imageset_t**)luaL_checkudata( L, 3, "imageset" );
  
  rl_map_t* self = rl_map_create( data, size, tileset, imageset );
  
  if ( self )
  {
    rl_map_t** ud = (rl_map_t**)lua_newuserdata( L, sizeof( rl_map_t* ) );
    *ud = self;
    
    lua_pushvalue( L, 2 );
    self->ud[ 0 ].i = luaL_ref( L, LUA_REGISTRYINDEX );
    
    lua_pushvalue( L, 3 );
    self->ud[ 1 ].i = luaL_ref( L, LUA_REGISTRYINDEX );
    
    if ( luaL_newmetatable( L, "map" ) != 0 )
    {
      lua_pushvalue( L, -1 );
      lua_setfield( L, -2, "__index" );
      luaL_setfuncs( L, methods, 0 );
    }
    
    lua_setmetatable( L, -2 );
    return 1;
  }
  
  return luaL_error( L, "error loading map" );
}

/*****************************************************************************/

static int openf( lua_State* L )
{
  lua_newtable( L );
  return 1;
}

void register_rl( lua_State* L, void* pack, retro_input_state_t* input_state_cb, retro_video_refresh_t* video_cb )
{
  static const luaL_Reg statics[] =
  {
    { "loadFile",      l_loadFile },
    { "create",        l_create },
    { "color",         l_color },
    { "clear",         l_clear },
    { "randomseed",    l_randomseed },
    { "randomize",     l_randomize },
    { "random",        l_random },
    { "stopSounds",    l_stopSounds },
    { "playMusic",     l_playMusic },
    { "getInputState", l_getInputState },
    { "presentVideo",  l_presentVideo },
    { "loadImage",     l_loadImage },
    { "loadImageSet",  l_loadImageSet },
    { "loadTileSet",   l_loadTileSet },
    { "newSprite",     l_newSprite },
    { "loadSound",     l_loadSound },
    { "loadMap",       l_loadMap },
    { NULL, NULL }
  };
  
  int top = lua_gettop( L );
  
  luaL_requiref( L, "rl", openf, 0 );
  
  lua_pushlightuserdata( L, pack );
  lua_pushlightuserdata( L, input_state_cb );
  lua_pushlightuserdata( L, video_cb );
  luaL_setfuncs( L, statics, 3 );
  
  lua_settop( L, top );
}
