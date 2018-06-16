#ifndef RL_PACK_H
#define RL_PACK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <rl_userdata.h>

#include <stdint.h>
#include <stddef.h>

typedef struct
{
  rl_userdata_t ud;
  
  void* opaque;
}
rl_stream_t;

int  rl_pack_init( const char* arg0, const char* organization, const char* app_name );
void rl_pack_done( void );

int  rl_pack_add( const char* path );

int  rl_pack_open( rl_stream_t* stream, const char* path, int write );
int  rl_pack_read( rl_stream_t* stream, void* buffer, unsigned* bytes );
int  rl_pack_write( rl_stream_t* stream, const void* buffer, unsigned bytes );
int  rl_pack_seek( rl_stream_t* stream, unsigned offset );
int  rl_pack_tell( rl_stream_t* stream, unsigned* pos );
int  rl_pack_size( rl_stream_t* stream, unsigned* bytes );
int  rl_pack_eof( rl_stream_t* stream );
void rl_pack_close( rl_stream_t* stream );

#ifdef __cplusplus
}
#endif

#endif /* RL_PACK_H */
