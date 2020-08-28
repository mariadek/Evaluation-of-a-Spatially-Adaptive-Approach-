#include "gdal.h"
#include "cpl_error.h"

typedef struct {
  GDALDatasetH hDataset;
  GDALDriverH hDriver;
  int nrows;
  int ncols;
  int nbands;
  GDALRasterBandH *hBand;
  double adfGeoTransform[6];
  /* adfGeoTransform[0] - top left x
     adfGeoTransform[1] - w-e pixel resolution
     adfGeoTransform[2] - 0
     adfGeoTransform[3] - top left y
     adfGeoTransform[4] - 0
     adfGeoTransform[5] - n-s pixel resolution (negative value)
  */
  double *noData; // for each band
  double **adfMinMax; // for each band
  float ***buffer;
} DATA;

DATA readRaster(char *);  // read input file, char * input filename

void writeOutput(GDALDriverH, char *, int **, int, int, int, int, double *, const char *);

int ***malloc3Dmatrix(int, int, int, double);
