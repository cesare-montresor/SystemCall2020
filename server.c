 /// @file server.c
/// @brief Contiene l'implementazione del SERVER.

#include <sys/types.h>
#include <sys/wait.h>

#include "defines.h"
#include "utils.h"
#include "config.h"

#include "ack_table.h"
#include "board.h"
#include "shared_memory.h"
#include "semaphore.h"
#include "fifo.h"
#include "position_file.h"
#include "device.h"
#include "signal.h"


void initSignals();
void receiveSignal(int signal);
void destroyServer();
void waitForKey();

PositionFile *pos_file;
AckTable* ack_table;
Board* board;
Device** device_list;

int main(int argc, char * argv[]) {
    
		//Check number of arguments
    printf("Reading args %*s",10,"\t");
		if(argc != 3){
			printf("Usage: %s msg_queue_key file_posizioni\n",argv[0]);
			exit(1);
		}
    
		//Read msg_queue's key
    key_t message_queue_key = atoi(argv[1]);
		if(message_queue_key <= 0){
			printf("The message_queue_key must be greater than zero!\n");
			exit(1);
		}
    printf("[ OK ]\n");

    clearScreen();
		
    //Read the file_posizioni
    printf("Load positions %*s",10+1," ");
    char* filename_pos = argv[2]; 
		pos_file = loadPositionFile(filename_pos);
    printf("[ OK ]\n");
    
		// ***** AckTable *****
    printf("AckTable setup %*s",10,"\t");
    AckTable* ack_table = createAckTable(message_queue_key, ACK_TABLE_SIZE, DEVICE_NUM);
    initAckTable(ack_table, 1);
    forkAckTableManager(ack_table);
    printf("[ OK ]\n");

    // ***** Board *****
    printf("Board setup %*s",10,"\t");
    board = createBoard(DEVICE_NUM + 1);
    initBoard(board);
    printf("[ OK ]\n");
    
    // ***** Device *****
    printf("Devices setup\n");
    device_list = malloc(sizeof(Device*) * DEVICE_NUM);
    
    for(int i=0; i < DEVICE_NUM; i++){
      PositionFileRow *device_row = pos_file->head[i].next;
      device_list[i] = createDevice(i, board, ack_table, device_row, pos_file->count);
      forkDevice(device_list[i]);
      printf("Device %d forked PID: %d\n", i, device_list[i]->pid);
    }
    printf("Devices setup %*s[ OK ]\n",10,"\t");
		
    // ***** Server *****
    printf("Running server\n");
    initSignals();
    long tick = 0;
    do{
			// Start device execution ( 0 -> 1 -> 2 -> 3 -> 4 )
      startTurnBoard(board); 
			
      sleep(1);
      //waitForKey();
      clearScreen();

      waitTurnBoard(board, DEVICE_NUM); //wait for last semaphore ( 5 )
			  
      printf("Ciclo: %ld\n",tick++);
      printBoard(board); 
      printAckTable(ack_table); 
      
    }while(TRUE);
		
		return 0;
}

void waitForKey(){
  char nl;
  scanf("%c",&nl);
}

void initSignals(){
		signal(SIGINT, receiveSignal);

		sigset_t mySet;

		//Initialize mySet to contain all signals
		sigfillset(&mySet);

		//Remove SIGINT from mySet
		sigdelset(&mySet, SIGINT);

		//Blocking all signals but SIGTERM
		sigprocmask(SIG_SETMASK, &mySet, 0);
		
}

void receiveSignal(int signal){
  if (signal == SIGINT){
    //Wait for children to die, lol
    while(wait(NULL) > 0){};

    int pid = getpid();
    printf("[ %d ]",pid);
    int found = 0;
    
    //TODO: propagate signal to ackmanager & devices
	  destroyServer();
    printf("\n");
		
    exit(0);
  }

}

void destroyServer(){
  destroyAckTable(ack_table);
  destroyBoard(board);
}

/*
DEFINE_NAME
StructName
functionName
var_name
*/
