#ifndef RL_PACK_H
#define RL_PACK_H

#include <rl_userdata.h>

#include <stdint.h>
#include <stddef.h>

typedef struct
{
  rl_userdata_t ud;
  
  const void* buffer;
  size_t      buffer_len;
}
rl_pack_t;

typedef struct
{
  rl_userdata_t ud;
  
  const void* contents;
  size_t      size;
}
rl_entry_t;

int     rl_pack_create( rl_pack_t* pack, const void* buffer, size_t buffer_len );
#define rl_pack_destroy( pack )

int     rl_find_entry( rl_entry_t* entry, const rl_pack_t* pack, const char* name );
#define rl_pack_entry_destroy( entry )

#endif /* RL_PACK_H */
