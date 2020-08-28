#!/usr/bin/env bash

# Path to the installed GDAL
GDAL_PATH = /usr/local/Cellar/gdal/3.1.2

INCLUDE_PATH = ${GDAL_PATH}/include
LIBS_PATH = ${GDAL_PATH}/lib
GDAL_LIB = -lgdal

main: main.c utils.c geomorphons.c
	gcc -I${INCLUDE_PATH} -L${LIBS_PATH} ${GDAL_LIB} main.c utils.c geomorphons.c -o geomorphons_modified -fopenmp

clean:
	rm main
