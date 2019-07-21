#include <rl_rand.h>

void rl_rand_create( rl_rand_t* rand, uint64_t seed )
{
  rand->seed = seed;
}

uint32_t rl_rand_rnd( rl_rand_t* rand )
{
  /*
  http://en.wikipedia.org/wiki/Linear_congruential_generator
  Newlib parameters
  */
  
  rand->seed = 6364136223846793005ULL * rand->seed + 1;
  return rand->seed >> 32;
}

int rl_rand_interval( rl_rand_t* rand, int min, int max )
{
  /* fixed point math */
  uint64_t const dividend = max - min + 1;
  return (int)( ( ( dividend * rl_rand_rnd( rand ) ) >> 32 ) + min );
}
