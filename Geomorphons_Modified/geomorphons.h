#include <math.h>
#include <stdlib.h>

#define RAD2DEG 57.29578

/* directions
 * 3|2|1
 * 4|0|8
 * 5|6|7 */

static int nextr[8] = { -1, -1, -1, 0, 1, 1, 1, 0 };
static int nextc[8] = { 1, 0, -1, -1, -1, 0, 1, 1 };

void geomorphons(float**, int***, int, double, double, int, int);

int binary(float);

unsigned int ternary_rotate(unsigned int);
