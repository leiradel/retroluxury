#ifndef RL_PACK_H
#define RL_PACK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

int  rl_pack_init( const char* arg0, const char* organization, const char* app_name );
void rl_pack_done( void );

int  rl_pack_add( const char* path );

#ifdef __cplusplus
}
#endif

#endif /* RL_PACK_H */
