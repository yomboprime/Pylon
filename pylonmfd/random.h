/*
        RANDOM.H: Random functions
        Juanjo'97
*/
#ifndef __RANDOM__
#define __RANDOM__

#include <stdlib.h>
#include <time.h>

inline void randomize(void) { srand((unsigned int)time(NULL)); }

#define frand ( ( (double)rand() ) / RAND_MAX )

#define range( x ) ( frand * x )

#define noise( x ) ( ( frand - 0.5 ) * x )

#endif
//__RANDOM__
