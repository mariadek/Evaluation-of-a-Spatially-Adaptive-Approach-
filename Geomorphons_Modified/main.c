/*
  MODULE:   geomorphons_modified

  AUTHOR:   Maria Dekavalla

  PURPOSE:  This program is a modification of a method, called geomorphons.

            This module can compute terrain parameters (binary and ternary) for
            every pixel as well as the number of higher and lower neighborhood
            directions for each pixel. This modification can compute these
            parameters by scanning the entire DEM with a lower computational
            cost. The scan radius for each direction increases until the
            absolute relief (i.e. absolute difference between the elevation of
            the central pixel and the pixel lying at a distance equal to the
            scan radius) reaches its maximum value.

            This program is free software: you can redistribute it and/or modify
            it under the terms of the GNU General Public License as published
            by the Free Software Foundation, either version 3 of the License, or
            (at your option) any later version.

            This program is distributed in the hope that it will be useful,
            but WITHOUT ANY WARRANTY; without even the implied warranty of
            MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
            GNU General Public License for more details.

            You should have received a copy of the GNU General Public License
            along with this program. If not, see <http://www.gnu.org/licenses/>.

            Dependencies: gdal, openmp(optional)

            Compilation: make

            Execution: ./geomorphons_modified <input> <output1> <output2> <output3>

            input: a raster Digital Elevation Model (DEM) file
            output1: ternary
            output2: number of higher neighborhood directions
            output3: number of lower neighborhood directions

            example: ./geomorphons_modified DEM.bil ternary.bil higher.bil lower.bil
*/


#include <stdio.h>
#include "utils.h"
#include "geomorphons.h"

int main(int argc, char **argv){

  char *input = argv[1];  // Input filename

  // Output filename
  char *output[3];

  output[0] = argv[2];
  output[1] = argv[3];
  output[2] = argv[4];

  DATA in;
  in = readRaster(input);

  int radius = in.nrows > in.ncols ? in.nrows : in.ncols;

  int noData = -9999;
  int ***outbuffer = malloc3Dmatrix(3, in.nrows, in.ncols, noData);

  geomorphons(in.buffer[0], outbuffer, radius, in.noData[0], in.adfGeoTransform[1], in.nrows, in.ncols);

  // create output files
  for(int a = 0; a < 3; a++)
    writeOutput(in.hDriver, output[a], outbuffer[a], in.nrows, in.ncols, noData,
        GDT_Int32, in.adfGeoTransform, GDALGetProjectionRef(in.hDataset));

  for(int a = 0; a < 3; a++)
    for(int r = 0; r < in.nrows; r++)
      free(outbuffer[a][r]);

  for(int a = 0; a < in.nbands; a++){
    for(int r = 0; r < in.nrows; r++)
      free(in.buffer[a][r]);
    free(in.adfMinMax[a]);
  }

  free(in.buffer); free(outbuffer); free(in.noData); free(in.hBand);
  free(in.adfMinMax);

  if(in.hDataset != NULL)
    GDALClose(in.hDataset);

  return 0;
}
