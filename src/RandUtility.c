#include "../include/RandUtility.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>

void resetSeed(void) { srand((unsigned)time(NULL)); }

int randInt(int low, int high) {
  return (int)(low + (1.0 * rand() / RAND_MAX) * (high - low + 1));
}
