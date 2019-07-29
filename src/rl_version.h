#ifndef RL_VERSION_H
#define RL_VERSION_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

extern const char* rl_gitstamp;
extern const char* rl_githash;

void rl_version_retroluxury( char* version, size_t size );
void rl_version_stb_image( char* version, size_t size );
void rl_version_physfs( char* version, size_t size );
void rl_version_soloud( char* version, size_t size );
void rl_version_libopenmpt( char* version, size_t size );

#ifdef __cplusplus
}
#endif

#endif /* RL_VERSION_H */
