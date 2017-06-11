/*
*
* PURPOSE:      Calculation of land surface parameters with Evans - Young
*
*               Available land surface parameters:
*                1.  slope gradient (G) (in degrees)
*                2.  profile (vertical) curvature (kv)(intersecting with the plane of the Z axis and aspect direction)
*                3.  tangential (horizontal) curvature(kh)
*                4.  minimal curvature (kmin)
*                5.  maximal curvature (kmax)
*
*
*
* COPYRIGHT:
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

#define MAX_FILENAME    256 /* Filename length limit */

#define TINY 1.0e-20      /* A small number */

#ifndef ABS
#define ABS(a)      ((a) > 0.0 ? (a) : -(a))
#endif

double nodata_value;             /* value for missing data */
double **A, **A_inverse;
double *col;
double *indx;
double sum;

void error(const char *);
void read_ascii(FILE *);
void write_ascii(FILE *, float**);
void show_progress(float);
void slope_evans();
void profc_evans();
void tangc_evans();
void minc_evans();
void maxc_evans();

/* ASCII Header */
int ncols;               /* number of columns */
int nrows;               /* number of rows */
double xllcorner;        /* western (left) x-coordinate - corner*/
double yllcorner;        /* southern (bottom) y-coordinate - corner */
double CellSize;         /* length of one side of a square cell */
double nodata_value;     /* value for missing data */

float **in_buffer;
float **out_buffer;

float prog;

int main(int argc, char **argv)
{
    FILE *fpin;         /* elevation values file pointer */
    FILE *fpin1;        /* prj file pointer */
    FILE *fpout;        /* output values file pointer */
    FILE *fpout1;       /* output prj file pointer */
    int filelen1, filelen2;
    char inpathname[50];
	char outpathname[50];
	char ch;

    if(argc != 4)
		error("Usage parameters: Input_DEM Output_LSP");

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


    if(strcmp(argv[3],"slope") == 0){
        printf(" Computation of slope\n");
        slope_evans();}
    else if (strcmp(argv[3],"profile") == 0){
        printf(" Computation of profile curvature\n");
        profc_evans();}
    else if(strcmp(argv[3],"tangential") == 0){
        printf(" Computation of tangential curvature\n");
        tangc_evans();}
    else if(strcmp(argv[3],"minimum") == 0){
        printf(" Computation of minimum curvature\n");
        minc_evans();}
    else if(strcmp(argv[3],"maximum") == 0){
        printf(" Computation of maximum curvature\n");
        maxc_evans();}
    else
        printf(" Menu: slope profile tangential minimum maximum ");


	/* WRITE OUTPUT */

	/* Check of file type */
	if(strcmp(pdest1, ext1) == 0){
		/* Write ASCII */
		fpout = fopen(argv[2], "w");
		write_ascii(fpout, out_buffer);

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

	return 0;
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
    fscanf(fp,"%s %lf", value, &CellSize);
    fscanf(fp,"%s %lf", value, &nodata_value);
	offset = ftell(fp);

	/* Display of header parameters */
	printf("\n ESRII ASCII Format DEM - Header Display:");
	printf("\n rows = %d", nrows);
	printf("\n columns = %d", ncols);
	printf("\n xllcorner = %lf", xllcorner);
	printf("\n yllcorner = %lf", yllcorner);
	printf("\n cellsize = %lf", CellSize);
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

void write_ascii(FILE *fp, float** out){

	int r, c;

	/* Write header */
	fprintf(fp, "ncols              %d\n", ncols);
	fprintf(fp, "nrows              %d\n", nrows);
	fprintf(fp, "xllcorner          %lf\n", xllcorner);
	fprintf(fp, "yllcorner          %lf\n", yllcorner);
	fprintf(fp, "cellsize           %lf\n", CellSize);
	fprintf(fp, "nodata_value       %lf\n", nodata_value);

	for(r = 0; r < nrows; r++){
		for(c = 0; c < ncols; c++)
			fprintf(fp, "%lf ", out[r][c]);
		fprintf(fp, "\n");
	}

	fclose(fp);
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


void slope_evans( )
{
	double P, Q, slope;
	double flt[9];
	int r, c;

    /* Allocate memory of output two dimensional array */
	out_buffer = malloc(sizeof(float) * nrows);
	for(r = 0; r < nrows; r++){
		out_buffer[r] = malloc(sizeof(float) * ncols);
	}

	for(r = 0; r < nrows; r++)
        for(c = 0; c < ncols; c++)
            out_buffer[r][c] = nodata_value;


        for(r = 1; r < nrows-1; r++){
            for(c = 1; c < ncols-1; c++){
                flt[0]=in_buffer[r-1][c-1];
                flt[1]=in_buffer[r-1][c];
                flt[2]=in_buffer[r-1][c+1];
                flt[3]=in_buffer[r][c-1];
                flt[4]=in_buffer[r][c];
                flt[5]=in_buffer[r][c+1];
                flt[6]=in_buffer[r+1][c-1];
                flt[7]=in_buffer[r+1][c];
                flt[8]=in_buffer[r+1][c+1];

                P=((flt[2]+flt[5]+flt[8])-(flt[0]+flt[3]+flt[6]))/(6*CellSize);
                Q=((flt[0]+flt[1]+flt[2])-(flt[6]+flt[7]+flt[8]))/(6*CellSize);

                slope = sqrt((P*P) + (Q*Q));


                /* degrees */
                out_buffer[r][c] = atan(slope) * 57.295779513082323;

            }
            prog = (float)(r)/(float)(nrows-2);
            show_progress(prog);
        }

}

void profc_evans()
{
	double P, Q, R, T, S, profc;
	double flt[9];
	int r, c;

    /* Allocate memory of output two dimensional array */
	out_buffer = malloc(sizeof(float) * nrows);
	for(r = 0; r < nrows; r++){
		out_buffer[r] = malloc(sizeof(float) * ncols);
	}

	for(r = 0; r < nrows; r++)
        for(c = 0; c < ncols; c++)
            out_buffer[r][c] = nodata_value;


        for(r = 1; r < nrows-1; r++){
            for(c = 1; c < ncols-1; c++){
                flt[0]=in_buffer[r-1][c-1];
                flt[1]=in_buffer[r-1][c];
                flt[2]=in_buffer[r-1][c+1];
                flt[3]=in_buffer[r][c-1];
                flt[4]=in_buffer[r][c];
                flt[5]=in_buffer[r][c+1];
                flt[6]=in_buffer[r+1][c-1];
                flt[7]=in_buffer[r+1][c];
                flt[8]=in_buffer[r+1][c+1];

       /*d */   P=((flt[2]+flt[5]+flt[8])-(flt[0]+flt[3]+flt[6]))/(6*CellSize);
       /*e */   Q=((flt[0]+flt[1]+flt[2])-(flt[6]+flt[7]+flt[8]))/(6*CellSize);
       /*a */   R=((flt[0]+flt[2]+flt[3]+flt[5]+flt[6]+flt[8])-2*(flt[1]+flt[4]+flt[7]))/(3*CellSize*CellSize);
       /*c */   S=((flt[2]+flt[6])-(flt[0]+flt[8]))/(4*CellSize*CellSize);
       /*b */   T=((flt[0]+flt[1]+flt[2]+flt[6]+flt[7]+flt[8])-2*(flt[3]+flt[4]+flt[5]))/(3*CellSize*CellSize);

                if( P == 0.0 && Q == 0.0 )
                    profc = nodata_value;
                else
                    profc = -((P*P*R) + 2 * (P*Q*S) + (Q*Q*T)) / (((P*P)+(Q*Q)) * pow((1+(P*P)+(Q*Q)) , 1.5));

                out_buffer[r][c] = profc;
            }
            prog = (float)(r)/(float)(nrows-2);
            show_progress(prog);
        }

}


void tangc_evans()
{
	double P, Q, R, T, S, tangc;
	double flt[9];
	int r, c;

    /* Allocate memory of output two dimensional array */
	out_buffer = malloc(sizeof(float) * nrows);
	for(r = 0; r < nrows; r++){
		out_buffer[r] = malloc(sizeof(float) * ncols);
	}

	for(r = 0; r < nrows; r++)
        for(c = 0; c < ncols; c++)
            out_buffer[r][c] = nodata_value;

        for(r = 1; r < nrows-1; r++){
            for(c = 1; c < ncols-1; c++){
                flt[0]=in_buffer[r-1][c-1];
                flt[1]=in_buffer[r-1][c];
                flt[2]=in_buffer[r-1][c+1];
                flt[3]=in_buffer[r][c-1];
                flt[4]=in_buffer[r][c];
                flt[5]=in_buffer[r][c+1];
                flt[6]=in_buffer[r+1][c-1];
                flt[7]=in_buffer[r+1][c];
                flt[8]=in_buffer[r+1][c+1];

       /*d */   P=((flt[2]+flt[5]+flt[8])-(flt[0]+flt[3]+flt[6]))/(6*CellSize);
       /*e */   Q=((flt[0]+flt[1]+flt[2])-(flt[6]+flt[7]+flt[8]))/(6*CellSize);
       /*a */   R=((flt[0]+flt[2]+flt[3]+flt[5]+flt[6]+flt[8])-2*(flt[1]+flt[4]+flt[7]))/(3*CellSize*CellSize);
       /*c */   S=((flt[2]+flt[6])-(flt[0]+flt[8]))/(4*CellSize*CellSize);
       /*b */   T=((flt[0]+flt[1]+flt[2]+flt[6]+flt[7]+flt[8])-2*(flt[3]+flt[4]+flt[5]))/(3*CellSize*CellSize);

                if( P == 0.0 && Q == 0.0 )
                    tangc = nodata_value;
                else
                    tangc = -((T*P*P)+(R*Q*Q)-2*(S*P*Q)) / (((Q*Q)+(P*P)) * sqrt(1+(P*P)+(Q*Q)));
                out_buffer[r][c] = tangc;
            }
            prog = (float)(r)/(float)(nrows-2);
            show_progress(prog);
        }
}

void minc_evans()
{
	double P, Q, R, T, S, minc;
	double flt[9];
	int r, c;

    /* Allocate memory of output two dimensional array */
	out_buffer = malloc(sizeof(float) * nrows);
	for(r = 0; r < nrows; r++){
		out_buffer[r] = malloc(sizeof(float) * ncols);
	}

	for(r = 0; r < nrows; r++)
        for(c = 0; c < ncols; c++)
            out_buffer[r][c] = nodata_value;

        for(r = 1; r < nrows-1; r++){
            for(c = 1; c < ncols-1; c++){
                flt[0]=in_buffer[r-1][c-1];
                flt[1]=in_buffer[r-1][c];
                flt[2]=in_buffer[r-1][c+1];
                flt[3]=in_buffer[r][c-1];
                flt[4]=in_buffer[r][c];
                flt[5]=in_buffer[r][c+1];
                flt[6]=in_buffer[r+1][c-1];
                flt[7]=in_buffer[r+1][c];
                flt[8]=in_buffer[r+1][c+1];

       /*d */   P=((flt[2]+flt[5]+flt[8])-(flt[0]+flt[3]+flt[6]))/(6*CellSize);
       /*e */   Q=((flt[0]+flt[1]+flt[2])-(flt[6]+flt[7]+flt[8]))/(6*CellSize);
       /*a */   R=((flt[0]+flt[2]+flt[3]+flt[5]+flt[6]+flt[8])-2*(flt[1]+flt[4]+flt[7]))/(3*CellSize*CellSize);
       /*c */   S=((flt[2]+flt[6])-(flt[0]+flt[8]))/(4*CellSize*CellSize);
       /*b */   T=((flt[0]+flt[1]+flt[2]+flt[6]+flt[7]+flt[8])-2*(flt[3]+flt[4]+flt[5]))/(3*CellSize*CellSize);

                minc = -R - T - sqrt(((R-T)*(R-T))+(S*S));

                out_buffer[r][c] = minc;
            }
            prog = (float)(r)/(float)(nrows-2);
            show_progress(prog);
        }
}

void maxc_evans()
{
	double P, Q, R, T, S, maxc;
	double flt[9];
	int r, c;

    /* Allocate memory of output two dimensional array */
	out_buffer = malloc(sizeof(float) * nrows);
	for(r = 0; r < nrows; r++){
		out_buffer[r] = malloc(sizeof(float) * ncols);
	}

	for(r = 0; r < nrows; r++)
        for(c = 0; c < ncols; c++)
            out_buffer[r][c] = nodata_value;

        for(r = 1; r < nrows-1; r++){
            for(c = 1; c < ncols-1; c++){
                flt[0]=in_buffer[r-1][c-1];
                flt[1]=in_buffer[r-1][c];
                flt[2]=in_buffer[r-1][c+1];
                flt[3]=in_buffer[r][c-1];
                flt[4]=in_buffer[r][c];
                flt[5]=in_buffer[r][c+1];
                flt[6]=in_buffer[r+1][c-1];
                flt[7]=in_buffer[r+1][c];
                flt[8]=in_buffer[r+1][c+1];

       /*d */   P=((flt[2]+flt[5]+flt[8])-(flt[0]+flt[3]+flt[6]))/(6*CellSize);
       /*e */   Q=((flt[0]+flt[1]+flt[2])-(flt[6]+flt[7]+flt[8]))/(6*CellSize);
       /*a */   R=((flt[0]+flt[2]+flt[3]+flt[5]+flt[6]+flt[8])-2*(flt[1]+flt[4]+flt[7]))/(3*CellSize*CellSize);
       /*c */   S=((flt[2]+flt[6])-(flt[0]+flt[8]))/(4*CellSize*CellSize);
       /*b */   T=((flt[0]+flt[1]+flt[2]+flt[6]+flt[7]+flt[8])-2*(flt[3]+flt[4]+flt[5]))/(3*CellSize*CellSize);

                maxc = -R - T + sqrt(((R-T)*(R-T))+(S*S));

                out_buffer[r][c] = maxc;
            }
            prog = (float)(r)/(float)(nrows-2);
            show_progress(prog);
        }
}


void error(const char *s) {
    printf("\nSlope reports: Error: <%s>.\n",s);
    exit(1);
}
