#ifndef RL_RAND_H
#define RL_RAND_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct {
  uint64_t seed;
}
rl_rand_t;

void     rl_rand_create( rl_rand_t* rand, uint64_t seed );
uint32_t rl_rand_rnd( rl_rand_t* rand );
int      rl_rand_interval( rl_rand_t* rand, int min, int max );

#ifdef __cplusplus
}
#endif

#endif /* RL_RAND_H */
