#include "utils.h"

int ***malloc3Dmatrix(int bands, int rows, int cols, double nodata){

  int ***m = (int ***) malloc(sizeof(int **) * bands);
  for(int a = 0; a < bands; a++){
    m[a] = (int **) malloc(sizeof(int *) * rows);
    for(int r = 0; r < rows; r++)
      m[a][r] = (int *) malloc(sizeof(int) * cols);
  }

  for(int a = 0; a < bands; a++)
    for(int r = 0; r < rows; r++)
      for(int c = 0; c < cols; c++)
        m[a][r][c] = nodata;

  return m;
}

DATA readRaster(char *input){

  DATA in;

  /* Opening the file */
  GDALAllRegister(); // Register all known drivers

  in.hDataset = GDALOpen(input, GA_ReadOnly);

  if(in.hDataset == NULL){
    CPLGetLastErrorMsg();
    exit(-1);
  }

  /* Getting Dataset Information */
  in.hDriver = GDALGetDatasetDriver(in.hDataset);

  if(GDALGetProjectionRef(in.hDataset) != NULL)

  if(GDALGetGeoTransform(in.hDataset, in.adfGeoTransform) == CE_None){
    if(in.adfGeoTransform[1] != - in.adfGeoTransform[5]){
      printf("w-e and n-s pixel size are not equal.\n");
      exit(-1);
    }
  }

  /* Fetching a Raster Band */
  in.nbands = GDALGetRasterCount(in.hDataset);  // number of bands
  if(in.nbands != 1){
    printf("DEM files have only one band.\n");
    exit(-1);
  }

  in.hBand = (GDALRasterBandH *) malloc(sizeof(GDALRasterBandH) * in.nbands);
  in.noData = (double *) malloc(sizeof(double) * in.nbands);
  in.adfMinMax = (double **) malloc(sizeof(double *) * in.nbands);
  for(int b = 0; b < in.nbands; b++)
    in.adfMinMax[b] = (double *) malloc(sizeof(double) * 2);

  for(int b = 0; b < in.nbands; b++){
    in.hBand[b] = GDALGetRasterBand(in.hDataset, b+1);
    in.ncols = GDALGetRasterBandXSize(in.hBand[b]);
    in.nrows = GDALGetRasterBandYSize(in.hBand[b]);

    int bGotNoValue;
    in.noData[b] = GDALGetRasterNoDataValue(in.hBand[b], &bGotNoValue);
    if(!bGotNoValue)
      in.noData[b] = -32767;

    int bGotMin, bGotMax;
    in.adfMinMax[b][0] = GDALGetRasterMinimum(in.hBand[b], &bGotMin);
    in.adfMinMax[b][1] = GDALGetRasterMaximum(in.hBand[b], &bGotMax);

    if(!(bGotMin && bGotMax))
      GDALComputeRasterMinMax(in.hBand[b],TRUE, in.adfMinMax[b]);
    //printf("Min=%.3f, Max=%.3f\n", in.adfMinMax[b][0], in.adfMinMax[b][1]);
  }


  in.buffer  = (float ***) malloc(sizeof(float **) * in.nbands);
  for(int b = 0; b < in.nbands; b++){
    in.buffer[b] = (float **) malloc(sizeof(float *) * in.nrows);
    for(int r = 0; r < in.nrows; r++)
      in.buffer[b][r] = (float *) malloc(sizeof(float) * in.ncols);
    }


  for(int b = 0; b < in.nbands; b++){
    for(int r = 0; r < in.nrows; r++){
      CPLErr e = GDALRasterIO(in.hBand[b], GF_Read, 0, r, in.ncols, 1, in.buffer[b][r], in.ncols, 1,
                  GDT_Float32, 0, 0);
    }
  }

  return in;
}

void writeOutput(GDALDriverH driver, char *out, int **buffer, int nrows, int ncols, int noData, int type, double *adfGeo, const char *proj){
  GDALDatasetH hDstDS;

  char **papszOptions = NULL;

  hDstDS = GDALCreate(driver, out, ncols, nrows, 1, type, papszOptions);

  GDALRasterBandH hBandOut = GDALGetRasterBand(hDstDS, 1);

  for(int r = 0; r < nrows; r++){
    CPLErr e = GDALRasterIO(hBandOut, GF_Write, 0, r, ncols, 1, buffer[r], ncols, 1,
                      type, 0, 0);
    }

  GDALSetGeoTransform(hDstDS, adfGeo);
  GDALSetProjection(hDstDS, proj);

  GDALSetRasterNoDataValue(hBandOut, noData);

  if(hDstDS != NULL)
    GDALClose(hDstDS);
}
