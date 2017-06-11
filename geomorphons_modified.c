/*
*
* MODULE:       geomorphons_modified
*
* AUTHOR:       Maria Dekavalla
*
* PURPOSE:      This program is a modification of the machine-vision technique, called
*               geomorphons.
*
*               This module can compute terrain patterns (binary and ternary) for every
*               pixel as well as the number of higher or lower neighbourhood directions
*               for each pixel. This modification can compute geomorphons from the
*               entire DEM scanning with a lower computational cost. The scan radius for
*               each direction increases until the absolute relief (i.e. absolute difference
*               between the elevations of the central pixel and the pixel lying at a distance
*               equal to the scan radius) reaches its maximum value.
*
*               This program is free software: you can redistribute it and/or modify
*               it under the terms of the GNU General Public License as published by
*               the Free Software Foundation, either version 3 of the License, or
*               (at your option) any later version.
*
*               This program is distributed in the hope that it will be useful,
*               but WITHOUT ANY WARRANTY; without even the implied warranty of
*               MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*               GNU General Public License for more details.
*
*               You should have received a copy of the GNU General Public License
*               along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef USE_OPENMP
#include <omp.h>
#endif

#define ABS(a)      ((a) > 0.0 ? (a) : -(a))


void read_ascii(FILE *);
void write_ascii(FILE *, int***, int a);
void error(const char *);
void geomorphons(int );
unsigned int ternary_rotate (unsigned int );
int binary(float);
void show_progress(float);


/* ASCII Header */
int ncols;               /* number of columns */
int nrows;               /* number of rows */
double xllcorner;        /* western (left) x-coordinate - corner*/
double yllcorner;        /* southern (bottom) y-coordinate - corner */
double cellsize;         /* length of one side of a square cell */
double nodata_value;     /* value for missing data */

float **in_buffer;
int ***out_buffer;

int radius;



int main(int argc, char **argv){
	FILE *fpin;         /* elevation values file pointer */
	FILE *fpin1;        /* prj file pointer */
	FILE *fpin2;        /* hdr file pointer */
	FILE *fpout;        /* output values file pointer */
	FILE *fpout1;       /* output prj file pointer */
	FILE *fpout2;       /* output hdr file pointer */

	char inpathname[50];
	char outpathname[50];
	int filelen1, filelen2;
	char ch;

	if(argc != 5)
		error("Usage parameters: Input_DEM Output_Ternary Output_Higher Output_Lower");

	/* Check of file type */
	char *pdest = strrchr(argv[1],'.');  /* input */
	char *pdest1 = strrchr(argv[2],'.'); /* output */
	char ext1[] = ".asc";

	/* Check input & output file type */
	if(strcmp(pdest, pdest1) != 0)
		error("Input and Output file type must be the same");

	if(strcmp(pdest, ext1) == 0){
		/* open ASCII file */
		fpin = fopen(argv[1], "r");
		if(fpin == NULL){
			error("The file doen't exist!");
		}
		/* find prj file */
		filelen1 = strlen(argv[1]);
		strncpy(inpathname, argv[1], filelen1-4);
		inpathname[filelen1-4] = '\0';
		strcat(inpathname,".prj");
		/* read prj file */
		fpin1 = fopen(inpathname, "r");
		if(fpin1 == NULL){
			error("DEM file is not projected!");
		}
		else{
			printf("\n DEM file is projected.\n");
			fclose(fpin1);
		}
		read_ascii(fpin);
	}

	else{
		error("Wrong file format! Only ESRI ASCII and HDR files are accepted!");
	}

	if(nrows > ncols)
        radius = nrows;
    else
        radius = ncols;

	geomorphons(radius);


	/* WRITE OUTPUT Ternary*/

	/* Check of file type */
	if(strcmp(pdest1, ext1) == 0){
		/* Write ASCII */
		fpout = fopen(argv[2], "w");
		write_ascii(fpout, out_buffer, 0);

		/* write prj file */

		/* open input prj file */
		filelen2 = strlen(argv[2]);
		strncpy(inpathname, argv[1], filelen1-4);
		inpathname[filelen1-4] = '\0';
		strcat(inpathname,".prj");
		fpin1 = fopen(inpathname, "r");

		strncpy(outpathname, argv[2], filelen2-4);
		outpathname[filelen2-4] = '\0';
		strcat(outpathname,".prj");
		fpout1 = fopen(outpathname, "w");

		ch = getc(fpin1);
		while(!feof(fpin1)){
			putc(ch, fpout1);
			ch = getc(fpin1);
		}
	}

	/* WRITE OUTPUT Higher*/

	/* Check of file type */
	if(strcmp(pdest1, ext1) == 0){
		/* Write ASCII */
		fpout = fopen(argv[3], "w");
		write_ascii(fpout, out_buffer, 1);

		/* write prj file */

		/* open input prj file */
		filelen2 = strlen(argv[3]);
		strncpy(inpathname, argv[1], filelen1-4);
		inpathname[filelen1-4] = '\0';
		strcat(inpathname,".prj");
		fpin1 = fopen(inpathname, "r");

		strncpy(outpathname, argv[3], filelen2-4);
		outpathname[filelen2-4] = '\0';
		strcat(outpathname,".prj");
		fpout1 = fopen(outpathname, "w");

		ch = getc(fpin1);
		while(!feof(fpin1)){
			putc(ch, fpout1);
			ch = getc(fpin1);
		}
	}

	/* WRITE OUTPUT Lower*/

	/* Check of file type */
	if(strcmp(pdest1, ext1) == 0){
		/* Write ASCII */
		fpout = fopen(argv[4], "w");
		write_ascii(fpout, out_buffer, 2);

		/* write prj file */

		/* open input prj file */
		filelen2 = strlen(argv[4]);
		strncpy(inpathname, argv[1], filelen1-4);
		inpathname[filelen1-4] = '\0';
		strcat(inpathname,".prj");
		fpin1 = fopen(inpathname, "r");

		strncpy(outpathname, argv[4], filelen2-4);
		outpathname[filelen2-4] = '\0';
		strcat(outpathname,".prj");
		fpout1 = fopen(outpathname, "w");

		ch = getc(fpin1);
		while(!feof(fpin1)){
			putc(ch, fpout1);
			ch = getc(fpin1);
		}
	}

	return 0;
}

void error(const char *s) {
    printf("\nGeomorphons computation reports: Error: <%s>.\n",s);
    exit(1);
}

void read_ascii(FILE *fp){
	char value[100];
	long offset;
	float elevation;
	int r, c;

	/* read ASCII header */
	fscanf(fp,"%s %d", value, &ncols);
    fscanf(fp,"%s %d", value, &nrows);
    fscanf(fp,"%s %lf", value, &xllcorner);
    fscanf(fp,"%s %lf", value, &yllcorner);
    fscanf(fp,"%s %lf", value, &cellsize);
    fscanf(fp,"%s %lf", value, &nodata_value);
	offset = ftell(fp);

	/* Display of header parameters */
	printf("\n ESRII ASCII Format DEM - Header Display:");
	printf("\n rows = %d", nrows);
	printf("\n columns = %d", ncols);
	printf("\n xllcorner = %lf", xllcorner);
	printf("\n yllcorner = %lf", yllcorner);
	printf("\n cellsize = %lf", cellsize);
	printf("\n nodata value = %lf\n", nodata_value);

	/* Allocate memory of input two dimensional array */
	in_buffer = malloc(sizeof(float) * nrows);
	for(r = 0; r < nrows; r++){
		in_buffer[r] = malloc(sizeof(float) * ncols);
	}

	/* Set pointer to the beginning of the elevation values */
	fseek(fp, offset, SEEK_SET);


	/* Read and store elevation values in buffer */
	for(r = 0; r < nrows; r++)
		for(c = 0; c < ncols; c++){
			fscanf(fp, "%f", &elevation);
			in_buffer[r][c] = elevation;
		}
}



void write_ascii(FILE *fp, int*** out, int a){

	int r, c;

	/* Write header */
	fprintf(fp, "ncols              %d\n", ncols);
	fprintf(fp, "nrows              %d\n", nrows);
	fprintf(fp, "xllcorner          %lf\n", xllcorner);
	fprintf(fp, "yllcorner          %lf\n", yllcorner);
	fprintf(fp, "cellsize           %lf\n", cellsize);
	fprintf(fp, "nodata_value       %lf\n", nodata_value);

	for(r = 0; r < nrows; r++){
		for(c = 0; c < ncols; c++)
			fprintf(fp, "%d ", out[a][r][c]);
		fprintf(fp, "\n");
	}

	fclose(fp);
}

void geomorphons(int radius)
{
     int r, c, a, b, i;
     int bin[8];
     int positive, negative, same;
     int ternary;
     int prog = 0;
     float E, NE, N, NW, W, SW, S, SE;
     float max_E, max_NE, max_N, max_NW, max_W, max_SW, max_S, max_SE;
     float phi_E, phi_NE, phi_N, phi_NW, phi_W, phi_SW, phi_S, phi_SE;
     float min_E, min_NE, min_N, min_NW, min_W, min_SW, min_S, min_SE;
     float psi_E, psi_NE, psi_N, psi_NW, psi_W, psi_SW, psi_S, psi_SE;
     float D_E, D_NE, D_N, D_NW, D_W, D_SW, D_S, D_SE;
     float diff_N0, diff_NE0, diff_E0, diff_SE0, diff_S0, diff_SW0, diff_W0, diff_NW0;

     double forms[9][9] = {
     /* minus ------------- plus ----------------*/
     /*       0   1   2   3   4   5   6   7   8  */
     /* 0 */ {1, 1, 1, 8, 8, 9, 9, 9, 10},
     /* 1 */ {1, 1, 8, 8, 8, 9, 9, 9, 0},
     /* 2 */ {1, 4, 6, 6, 7, 7, 9, 0, 0},
     /* 3 */ {4, 4, 6, 6, 6, 7, 0, 0, 0},
     /* 4 */ {4, 4, 5, 6, 6, 0, 0, 0, 0},
     /* 5 */ {3, 3, 5, 5, 0, 0, 0, 0, 0},
     /* 6 */ {3, 3, 3, 0, 0, 0, 0, 0, 0},
     /* 7 */ {3, 3, 0, 0, 0, 0, 0, 0, 0},
     /* 8 */ {2, 0, 0, 0, 0, 0, 0, 0, 0},
     };

    /* legend:
    1,  flat
    2,  peak, summit
    3,  ridge
    4,  shoulder
    5,  convex
    6,  slope
    7,  concave
    8,  footslope
    9,  valley
    10,  pit, depression
    0,  error, impossible
    */

    out_buffer = malloc(sizeof(int) * 3);
	for(a = 0; a < 3; a++){
	    out_buffer[a] = malloc(sizeof(int) * nrows);
	    for(r = 0; r < nrows; r++)
            out_buffer[a][r] = malloc(sizeof(int) * ncols);}


    for(a = 0; a < 3; a++)
     for(r = 0; r < nrows; r++)
        for(c = 0; c < ncols; c++)
            out_buffer[a][r][c] = nodata_value;


   /* omp_set_num_threads(6); */

    # pragma omp parallel for default(none) shared(prog, nrows, ncols, radius, in_buffer, \
      out_buffer, nodata_value, cellsize, xllcorner, yllcorner)\
      private(r,c,a, E, NE, N, NW, W, SW, S, SE, \
      diff_N0, diff_NE0, diff_E0, diff_SE0, diff_S0, diff_SW0, diff_W0, diff_NW0, max_E, max_NE, \
      max_N, max_NW, max_W, max_SW, max_S, max_SE, min_E, min_NE, min_N, min_NW, min_W, min_SW, \
      min_S, min_SE, phi_E, phi_NE, phi_N, phi_NW, phi_W, phi_SW, phi_S, phi_SE, psi_E, psi_NE, \
      psi_N, psi_NW, psi_W, psi_SW, psi_S, psi_SE, D_E, D_NE, D_N, D_NW, D_W, D_SW, D_S, D_SE, \
      ternary, i, bin) reduction(+:positive, negative, same)
    for(r = 1; r < nrows-1; r++){
        for(c = 1; c < ncols-1; c++){
            max_E = -90;
            max_NE = -90;
            max_N = -90;
            max_NW = -90;
            max_W = -90;
            max_SW = -90;
            max_S = -90;
            max_SE = -90;
            min_E = 90;
            min_NE = 90;
            min_N = 90;
            min_NW = 90;
            min_W = 90;
            min_SW = 90;
            min_S = 90;
            min_SE = 90;
            diff_N0 = 0;
            diff_NE0 = 0;
            diff_E0 = 0;
            diff_SE0 = 0;
            diff_S0 = 0;
            diff_SW0 = 0;
            diff_W0 = 0;
            diff_NW0 = 0;
            ternary = 0;
            /* North Direction */
            for(a = 0; a <= radius; a++){
                if(r - a >= 0 && in_buffer[r][c] != nodata_value && in_buffer[r-a][c] != nodata_value && ABS(in_buffer[r-a][c] - in_buffer[r][c]) > diff_N0 ){
                    N = atan((in_buffer[r-a][c] - in_buffer[r][c])/(ABS(a)*cellsize)) * 57.295779513082323;
                    if(N >= max_N)
                        max_N = N;
                    if(N <= min_N)
                        min_N = N;
                diff_N0 = ABS(in_buffer[r-a][c] - in_buffer[r][c]);
                }
            }
            /* South Direction */
            for(a = 0; a <= radius; a++){
                  if(r + a < nrows && in_buffer[r][c] != nodata_value && in_buffer[r+a][c] != nodata_value && ABS(in_buffer[r+a][c] - in_buffer[r][c]) > diff_S0){
                    S = atan((in_buffer[r+a][c] - in_buffer[r][c])/(ABS(a)*cellsize)) * 57.295779513082323;
                    if(S >= max_S)
                        max_S = S;
                    if(S <= min_S)
                        min_S = S;
                    diff_S0 = ABS(in_buffer[r+a][c] - in_buffer[r][c]);
                  }
            }
            /* East Direction */
            for(a = 0; a <= radius; a++){
                if(c + a < ncols && in_buffer[r][c] != nodata_value && in_buffer[r][c+a] != nodata_value  && ABS(in_buffer[r][c+a] - in_buffer[r][c]) > diff_E0){
                    E = atan((in_buffer[r][c+a] - in_buffer[r][c])/(ABS(a)*cellsize)) * 57.295779513082323;
                    if(E >= max_E)
                        max_E = E;
                    if(E <= min_E)
                        min_E = E;
                    diff_E0 = ABS(in_buffer[r][c+a] - in_buffer[r][c]);
                }
            }
            /* West Direction */
             for(a = 0; a <= radius; a++){
                 if(c - a >= 0 && in_buffer[r][c] != nodata_value && in_buffer[r][c-a] != nodata_value && ABS(in_buffer[r][c-a] - in_buffer[r][c]) > diff_W0){
                    W = atan((in_buffer[r][c-a] - in_buffer[r][c])/(ABS(a)*cellsize)) * 57.295779513082323;
                    if(W >= max_W)
                        max_W = W;
                    if(W <= min_W)
                        min_W = W;
                    diff_W0 = ABS(in_buffer[r][c-a] - in_buffer[r][c]);
                  }
              }
              /* NorthWest Direction */
              for(a = 0; a <= radius; a++){
                 if(c - a >= 0 && r - a >= 0 && in_buffer[r][c] != nodata_value && in_buffer[r-a][c-a] != nodata_value && ABS(in_buffer[r-a][c-a] - in_buffer[r][c]) > diff_NW0){
                    NW = atan((in_buffer[r-a][c-a] - in_buffer[r][c])/(ABS(a)*sqrt(2)*cellsize)) * 57.295779513082323;
                    if(NW >= max_NW)
                        max_NW = NW;
                    if(NW <= min_NW)
                        min_NW = NW;
                    diff_NW0 = ABS(in_buffer[r-a][c-a] - in_buffer[r][c]);
                    }
                }
                /* SouthEast Direction */
                for(a = 0; a <= radius; a++){
                  if(c + a < ncols && r + a < nrows && in_buffer[r][c] != nodata_value && in_buffer[r+a][c+a] != nodata_value && ABS(in_buffer[r+a][c+a] - in_buffer[r][c]) > diff_SE0){
                    SE = atan((in_buffer[r+a][c+a] - in_buffer[r][c])/(ABS(a)*sqrt(2)*cellsize)) * 57.295779513082323;
                    if(SE >= max_SE)
                        max_SE = SE;
                    if(SE <= min_SE)
                        min_SE = SE;
                    diff_SE0 = ABS(in_buffer[r+a][c+a] - in_buffer[r][c]);
                    }
                }
                /* SouthWest Direction */
                 for(a = 0; a <= radius; a++){
                    if(r + a < nrows && c - a >= 0 && in_buffer[r][c] != nodata_value && in_buffer[r+a][c-a] != nodata_value && ABS(in_buffer[r+a][c-a] - in_buffer[r][c]) > diff_SW0){
                    SW = atan((in_buffer[r+a][c-a] - in_buffer[r][c])/(ABS(a)*sqrt(2)*cellsize)) * 57.295779513082323;
                    if(SW >= max_SW)
                        max_SW = SW;
                    if(SW <= min_SW)
                        min_SW = SW;
                    diff_SW0 = ABS(in_buffer[r+a][c-a] - in_buffer[r][c]);
                    }
                 }
                 /* NorthEast Direction */
                 for(a = 0; a <= radius; a++){
                    if(r - a >= 0 && c + a < ncols&& in_buffer[r][c] != nodata_value && in_buffer[r-a][c+a] != nodata_value && ABS(in_buffer[r-a][c+a] - in_buffer[r][c]) > diff_NE0){
                    NE = atan((in_buffer[r-a][c+a] - in_buffer[r][c])/(ABS(a)*sqrt(2)*cellsize)) * 57.295779513082323;
                    if(NE >= max_NE)
                        max_NE = NE;
                    if(NE <= min_NE)
                        min_NE = NE;
                    diff_NE0 = ABS(in_buffer[r-a][c+a] - in_buffer[r][c]);
                    }
                 }


        phi_E = 90 - max_E;
        phi_N = 90 - max_N;
        phi_S = 90 - max_S;
        phi_W = 90 - max_W;
        phi_NW = 90 - max_NW;
        phi_SW = 90 - max_SW;
        phi_NE = 90 - max_NE;
        phi_SE = 90 - max_SE;

        psi_E = 90 + min_E;
        psi_N = 90 + min_N;
        psi_S = 90 + min_S;
        psi_W = 90 + min_W;
        psi_NW = 90 + min_NW;
        psi_SW = 90 + min_SW;
        psi_NE = 90 + min_NE;
        psi_SE = 90 + min_SE;

        D_E = psi_E - phi_E;
        bin[0] = binary(D_E);
        D_SE = psi_SE - phi_SE;
        bin[1] = binary(D_SE);
        D_S = psi_S - phi_S;
        bin[2] = binary(D_S);
        D_SW = psi_SW - phi_SW;
        bin[3] = binary(D_SW);
        D_W = psi_W - phi_W;
        bin[4] = binary(D_W);
        D_NW = psi_NW - phi_NW;
        bin[5] = binary(D_NW);
        D_N = psi_N - phi_N;
        bin[6] = binary(D_N);
        D_NE = psi_NE - phi_NE;
        bin[7] = binary(D_NE);


        for(i = 0; i < 8; i++){
            int power = 7 - i;
            ternary += bin[i] * pow(3, power);
        }



        positive = 0;
        negative = 0;
        same = 0;

        for(i = 0; i < 8; i++){
            if(bin[i] == 2)
                positive++; /* number of higher neighbourhood directions */
            if(bin[i] == 1)
                same++;
            if(bin[i] == 0)
                negative++; /* number of lower neighbourhood directions */
            }


       if(in_buffer[r][c] == nodata_value){
            for(a = 0; a < 3; a++)
                out_buffer[a][r][c] = nodata_value;
            }
       else{

       out_buffer[0][r][c] = ternary_rotate (ternary);
       out_buffer[1][r][c] = positive;
       out_buffer[2][r][c] = negative; }

        }


       #pragma omp atomic
       prog++;

       #pragma omp critical
       show_progress((float)(prog)/(float)(nrows-2));

     }


    for(r = 0; r < nrows; r++)
        free(in_buffer[r]);
    free(in_buffer);
}


int binary(float diff)
{
    int binary;

    /* lower 0; equal 1; higher 2 */

    if (diff > 0)
        binary = 2;  /* nadir is larger than zenith - higher neighbors */
    if(ABS(diff) == 0)
        binary = 1;  /* nadir is same as zenith - equal neighbors */
    if(diff < 0)
        binary = 0;  /* nadir is smaller than zenith - lower neighbors */

    return binary;
}

unsigned int ternary_rotate (unsigned int value)
{
/* this function is taken from the original geomorphon module
 *
 * this function returns rotated and mirrored
 * ternary code from any number
 * function is used to create lookup table with original
 * terrain patterns (6561) and its rotated and mirrored counterparts (498)*/

unsigned char pattern[8];
unsigned char rev_pattern[8];
unsigned char tmp_pattern[8];
unsigned char tmp_rev_pattern[8];
unsigned int code=10000, tmp_code, rev_code=10000, tmp_rev_code;
int power=1;
int i,j,k;
int res;

for (i=0;i<8;i++) {
	pattern[i]=value%3;
	rev_pattern[7-i]=value%3;
	value/=3;
}

for (j=0;j<8;j++) {
	power=1;
	tmp_code=0;
	tmp_rev_code=0;
	for (i=0;i<8;i++) {
		k=(i-j)<0 ? j-8 : j;
		tmp_pattern[i]=pattern[i-k];
		tmp_rev_pattern[i]=rev_pattern[i-k];
		tmp_code+=tmp_pattern[i]*power;
		tmp_rev_code+=tmp_rev_pattern[i]*power;
		power*=3;
	}
	code=tmp_code<code ? tmp_code :code;
	rev_code=tmp_rev_code<rev_code ? tmp_rev_code :rev_code;
}
return code<rev_code ? code : rev_code;
}

void show_progress(float progress){

int i, barWidth, pos;

if (progress <= 1.0) {
    barWidth = 70;

    printf(" ");
    pos = barWidth * progress;
    for (i = 0; i < barWidth; ++i) {
        if (i < pos)
            printf("*");
        else if (i == pos)
            printf(">");
        else printf( " ");
    }
    printf("%\r %d %% ", (int)(progress * 100.0));


    progress += 0.01;
    }
}

