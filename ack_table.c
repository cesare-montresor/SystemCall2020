#include "ack_table.h"
#include <errno.h>
#include "signal.h"

//Private functions
void runAckTableManager(AckTable *ack_table);
void submitCompletedAck(AckTable *ack_table);

void receiveAckManager(int signal);
void initSignalsAckManager();

AckTable* createAckTable(key_t msg_queue_key, int num_rows, int dev_num){
  // Create ACKTable ds
  AckTable *ack_table = malloc( sizeof(AckTable) );
  ack_table->dev_num = dev_num;
  ack_table->size = num_rows;
  ack_table->pid = -1;
  ack_table->msg_queue_key = msg_queue_key;

  // Create SHM & SEM
  ack_table->ack_mem = createMutexSharedMemory(sizeof(AckTableRow) * num_rows, 1);
  
	// Init message queue  
  ack_table->msg_queue_id = msgget(ack_table->msg_queue_key, IPC_CREAT | S_IRUSR | S_IWUSR);
	if(ack_table->msg_queue_id == -1)
		errExit("Ack_table_Msgget failed!");

  return ack_table;
}

void destroyAckTable(AckTable* ack_table){
  destroyMutexSharedMemory(ack_table->ack_mem);

	//Destroy msg_queue
	if (msgctl(ack_table->msg_queue_id, IPC_RMID, NULL) == -1)
            errExit("msgctl failed");
						
  free(ack_table);
}

//Ack Table data structure
void initAckTable(AckTable *ack_table, int sem_value ){
  //Init mutx_shared memory semaphore to value
  setAllSemaphore(ack_table->ack_mem->semaphore, (int[]){sem_value});
  
  //cleanup ack ack_list ( message_id = 0 => empty )
  AckTableList* ack_list = (AckTableList*) ack_table->ack_mem->mem->ptr;
  for(int i = 0; i < ack_table->size ; i += 1){
    ack_list->rows[i].message_id = 0;
  }
}

AckTable *forkedAckTable;

int forkAckTableManager(AckTable *ack_table){
  pid_t pid = fork();

  //Failed
  if (pid < 0){ 
		//return FALSE; 
		errExit("Ack_table_Fork failed!");
	}

  //Parent
  if(pid > 0){ 
    ack_table->pid = pid;
    return TRUE; 
  }

  //Child (Ack Manager)
  //initSignalsAckManager();
  forkedAckTable = ack_table;
	
  ack_table->pid = getpid();
  runAckTableManager(ack_table);
  
  exit(0);
}

void runAckTableManager(AckTable *ack_table){
	initSignalsAckManager();

  while(TRUE){
    submitCompletedAck(ack_table);
    sleep(5);
  }
}

void requestAckTable(AckTable *ack_table){
  waitSemaphore(ack_table->ack_mem->semaphore, 0);
}

void releaseAckTable(AckTable *ack_table){
  signalSemaphore(ack_table->ack_mem->semaphore, 0);
}

void printAckTable(AckTable *ack_table){
  requestAckTable(ack_table);

  printf("\n----------------ACK msg_queue_id = %d -----------\n",ack_table->msg_queue_id);
  AckTableList* ack_list = (AckTableList*) ack_table->ack_mem->mem->ptr;
  AckTableRow *ack_row;

  for(int slot = 0; slot < ack_table->size ; slot += ack_table->dev_num){
    ack_row = &ack_list->rows[slot];
    
    if (ack_row->message_id != 0 ) {
      printf("%2d | %d => ",slot, ack_row->message_id );
      for (int i=0;i<DEVICE_NUM;i++){
        ack_row = &ack_list->rows[slot + i];
        if (ack_row->message_id != 0 ) {
          printf("%d ",ack_row->pid_reciever);
        }
      }
      printf("\n");
    }    
  }
  printf("---------------------------------------------\n");
  
  releaseAckTable(ack_table);
}

int searchForMsgInAckTable(AckTable *ack_table, int message_id, pid_t *devices_pid){
	int cnt = 0;
	for(int j = 0; j < DEVICE_NUM; j++){
		devices_pid[j] = 0;
	}

  requestAckTable(ack_table);

  AckTableList* ack_list = (AckTableList*) ack_table->ack_mem->mem->ptr;
  AckTableRow *ack_row;
  int slot;

  for(slot = 0; slot < ack_table->size ; slot += ack_table->dev_num){
    ack_row = &ack_list->rows[slot];
    if(ack_row->message_id == message_id){
      break;
    }
  }

  for(int i = 0; i < DEVICE_NUM; i++){
    ack_row = &ack_list->rows[i+slot];
    if (ack_row->message_id == message_id){
      devices_pid[i] = ack_row->pid_reciever;
      cnt++;
    }
  }
  
  releaseAckTable(ack_table);
  return cnt;
}

void addAckTableRow(AckTable *ack_table, AckTableRow *new_row){

  //printf("Request ack table!\n");
  requestAckTable(ack_table);
  AckTableList* ack_list = (AckTableList*) ack_table->ack_mem->mem->ptr;
  int free_slot = -1;
  int found_slot = -1;
  int slot_size = ack_table->dev_num;
	
  //Search end of each slot (table_size / slot_size)
  for(int slot = 0; slot < ack_table->size ; slot += slot_size ){
    //first row
    AckTableRow *row =  &ack_list->rows[slot];

    //Found free space
    if (free_slot == -1 && row->message_id == 0){
      free_slot = slot;
      continue;
    }

    if (row->message_id == new_row->message_id){
      //found_slot = slot;
      for(found_slot = slot ; found_slot < slot+slot_size; found_slot++ ){
        if(ack_list->rows[found_slot].message_id == 0){
          printf("oldAck(%d, %d, %d )",row->message_id, slot, found_slot-slot );
          ack_list->rows[found_slot].pid_sender = new_row->pid_sender;
          ack_list->rows[found_slot].pid_reciever = new_row->pid_reciever;
          ack_list->rows[found_slot].message_id = new_row->message_id;
          ack_list->rows[found_slot].timestamp = new_row->timestamp;
          break; //Message added 
        }
      }
      break; 
    }
  }

  if (found_slot == -1 && free_slot != -1){
    printf("newAck(%d)", free_slot );
    ack_list->rows[free_slot].pid_sender = new_row->pid_sender;
    ack_list->rows[free_slot].pid_reciever = new_row->pid_reciever;
    ack_list->rows[free_slot].message_id = new_row->message_id;
    ack_list->rows[free_slot].timestamp = new_row->timestamp;
  }
  
  releaseAckTable(ack_table);
}

void submitCompletedAck(AckTable *ack_table){
  //printf("Running the ack_manager");
  requestAckTable(ack_table);

  AckTableList* ack_list = (AckTableList*) ack_table->ack_mem->mem->ptr;

  //AckTableRow *ack_row;
  MessageAckList msgAckList;
  //printf("ACK MNGR 1\n");

  int slot_size = ack_table->dev_num;

  //Search end of each slot (table_size / slot_size)
  for(int slot = 0; slot < ack_table->size ; slot += slot_size ){
    
		//Checking last entry of the slot
    int slot_last = slot + (slot_size - 1);
    
    AckTableRow *last_ack_row = &ack_list->rows[slot_last];
    

    //Found a full slot (last is full)
    if(last_ack_row->message_id > 0){
      printf("ACK Manager cleanup & submit slot %d for msg id %d \n",slot, last_ack_row->message_id );  

      //Compose message
      msgAckList.type = last_ack_row->message_id;  
      for (int j = 0; j < slot_size ; j++ ){
        int idx = slot + j;
        AckTableRow* row = &ack_list->rows[idx];
        msgAckList.rows[j].pid_sender = row->pid_sender;
        msgAckList.rows[j].pid_reciever = row->pid_reciever;
        msgAckList.rows[j].timestamp = row->timestamp;

        //Cleanup row
        row->message_id = 0;
      }
      
			//Printing msgAcklist, that i am about to Send
      for (int j = 0; j < slot_size ; j++ ){
        printf("[%d]Snd = %d ",j, msgAckList.rows[j].pid_sender);
        printf("Rcv = %d ",msgAckList.rows[j].pid_reciever);
        printf("Time = %d",(int)msgAckList.rows[j].timestamp);
				printf("\n");
      }
			

      //Submit message to client
      size_t msg_size = sizeof(struct MessageAckList) - sizeof(long); 

      if( msgsnd(ack_table->msg_queue_id, &msgAckList, msg_size,0) == -1) {
        errExit("msgsnd failed");
      };
    }
  }

  releaseAckTable(ack_table);
}

void initSignalsAckManager(){
		signal(SIGINT, receiveAckManager);

		sigset_t mySet;

		//Initialize mySet to contain all signals
		sigfillset(&mySet);

		//Remove SIGINT from mySet
		sigdelset(&mySet, SIGINT);

		//Blocking all signals but SIGTERM
		sigprocmask(SIG_SETMASK, &mySet, 0);		
}

void receiveAckManager(int signal){
  if (signal == SIGINT){
    printf("Ciao sono l'ack manager pid: %d\n",getpid());
	  destroyAckTable(forkedAckTable);
    exit(0);
  }
}

