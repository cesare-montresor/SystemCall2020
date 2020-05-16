#pragma once

#define  PRJ_ID 42

//Devices
#define DEVICE_NUM 5
#define DEVICE_QUEUE_PATH "/tmp/dev_fifo.%d"
#define POS_ROW_LENGTH 21 
#define LOC_MSG_LEN 10

//Board
#define BOARD_GRID_SIZE 10
#define BOARD_SHM_PATH "/tmp/board_mem"
#define BOARD_SHM_PRJ PRJ_ID

//Ack Manager
#define ACK_TABLE_SIZE 100
#define ACK_TABLE_SHM_PATH "/tmp/ack_table"
#define ACK_TABLE_SHM_PRJ PRJ_ID

//Client
#define CLIENT_OUT_PATH "./output/msg_id_%ld.txt"
#define MSG_TIME_FORMAT "%Y-%m-%d %H:%M:%S"