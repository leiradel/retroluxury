#ifndef RL_BASE64_H
#define RL_BASE64_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

size_t  rl_base64_decode( const void* buffer, size_t length, void* output );
#define rl_base64_decode_inplace( buffer, length ) do { return rl_base64_decode( ( buffer ), ( length ), ( buffer ) ); } while ( 0 )

#ifdef __cplusplus
}
#endif

#endif /* RL_BASE64_H */
