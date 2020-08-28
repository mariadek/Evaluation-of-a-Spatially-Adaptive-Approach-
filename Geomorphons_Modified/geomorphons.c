#include "geomorphons.h"

void geomorphons(float **in, int ***out, int radius, double noData, double cellsize, int nrows, int ncols){

  # pragma omp parallel for
  for(int r = 1; r < nrows-1; r++){
    for(int c = 1; c < ncols-1; c++){
      if(in[r][c] != noData){
        int ternary = 0, higher = 0, lower = 0;
        float angle[8], max[8], phi[8], psi[8], min[8], diff[8], diff_c[8], bin[8];
        for(int d = 0; d < 8; d++){
          max[d] = -90, min[d] = 90, diff[d] = 0;
          for(int a = 0; a <= radius; a++){  // function 1 with loop a
            int r1 = a * nextr[d];
            int c1 = a * nextc[d];
            float d1 = sqrt(nextr[d] * nextr[d] + nextc[d] * nextc[d]);
            if(r + r1 >= 0 && r + r1 < nrows && c + c1 >= 0 && c + c1 < ncols){
              if(in[r+r1][c+c1] != noData){
                diff_c[d] = in[r+r1][c+c1] - in[r][c];
                if(fabsf(diff_c[d]) > diff[d]){
                  angle[d] = atan(diff_c[d] /(a * d1 * cellsize)) * RAD2DEG;
                  if(angle[d] >= max[d])
                    max[d] = angle[d];
                  if(angle[d] < min[d])
                    min[d] = angle[d];
                  diff[d] = fabsf(diff_c[d]);
                }
              }
            }
          }

          phi[d] = 90 - max[d];
          psi[d] = 90 + min[d];
          bin[d] = binary(psi[d] - phi[d]);
          int power = 7 - d;
          ternary += bin[d] * pow(3, power);

          if(bin[d] == 2)
            ++higher;   // number of higher neighborhood directions
          else if(bin[d] == 0)
            ++lower;   // number of lower neighborhood directions
        }

        out[0][r][c] = ternary_rotate(ternary);
        out[1][r][c] = higher;
        out[2][r][c] = lower;

      }
    }
  }
}

int binary(float diff){

  int binary = 0;

  /* lower  0
     equal  1
     higher 2 */

  if(diff > 0)
    binary = 2; // nadir is larger than zenith - higher neighbors
  else if(fabsf(diff) == 0)
    binary = 1; // nadir same as zenith - equal neighbors
  else if(diff < 0)
    binary = 0;

  return binary; // nadir is smaller than zenith - lower neighbors
}

unsigned int ternary_rotate(unsigned int value){
  unsigned char pattern[8];
  unsigned char rev_pattern[8];
  unsigned char tmp_pattern[8];
  unsigned char tmp_rev_pattern[8];
  unsigned int code = 10000, tmp_code, rev_code = 10000, tmp_rev_code;
  int power = 1;
  int i, j, k;
  int res;

  for(i = 0; i < 8; i++){
    pattern[i] = value % 3;
    rev_pattern[7-i] = value % 3;
    value /= 3;
  }

  for(j = 0; j < 8; j++){
    power = 1;
    tmp_code = 0;
    tmp_rev_code = 0;
    for(i = 0; i < 8; i++){
      k = (i - j) < 0 ? j - 8 : j;
      tmp_pattern[i] = pattern[i-k];
      tmp_rev_pattern[i] = rev_pattern[i-k];
      tmp_code += tmp_pattern[i] * power;
      tmp_rev_code += tmp_rev_pattern[i] * power;
      power *= 3;
     }
    code = tmp_code < code ? tmp_code : code;
    rev_code = tmp_rev_code < rev_code ? tmp_rev_code : rev_code;
    }
    return code < rev_code ? code : rev_code;
}
