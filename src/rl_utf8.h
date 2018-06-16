#ifndef RL_UTF8_H
#define RL_UTF8_H

#ifdef __cplusplus
extern "C" {
#endif

#define RL_UNICODE_UNKNOWN 0xfffd

int rl_utf8_decode( const char** utf8 );

#ifdef __cplusplus
}
#endif

#endif /* RL_UTF8_H */
