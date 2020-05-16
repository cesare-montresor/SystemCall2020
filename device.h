#pragma once



#include "defines.h"
#include "board.h"
#include "ack_table.h"
#include "position_file.h"
#include "message.h"
#include "fifo.h"

#ifndef _DEVICE_HH
#define _DEVICE_HH 


struct Device{
  int num;
  pid_t pid;
  Board *board;
  AckTable *ack_table;
  int x;
  int y;
  PositionFileRow *pos;
  int pos_count;

  MessageList msg_list; 
  int turn_num;
  Fifo *fifo;
};
typedef struct Device Device;

Device* createDevice(int num, Board* board, AckTable* ack_table, PositionFileRow *pos, int pos_count);

void destroyDevice(Device *device);
int forkDevice(Device *device);

#endif