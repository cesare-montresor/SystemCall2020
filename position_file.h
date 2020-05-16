#pragma once

#ifndef _POSITION_FILE_HH
#define _POSITION_FILE_HH


#include "defines.h"


struct PositionFileRow{
  int coords[2];
  struct PositionFileRow *next;
};
typedef struct PositionFileRow  PositionFileRow;

struct PositionFile{
  char path[255];
  int count;
  PositionFileRow head[DEVICE_NUM];
  PositionFileRow* tail[DEVICE_NUM];
};
typedef struct PositionFile  PositionFile;

PositionFile* loadPositionFile(char* path);


#endif