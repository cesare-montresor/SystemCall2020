#include "device.h"
#include "signal.h"
#include <time.h>

//Private Methods
void runDevice(Device *device);
void initFifoDevice(Device *device);

void submitDevice(Device *device);
void reciveDevice(Device *device);
void moveDevice(Device *device);

void initSignalsDevice();
void receiveSignalDevice(int signal);

int removeDevicesRecived(pid_t *devices_in_range, pid_t *devices_received);


Device* createDevice(int num, Board* board, AckTable* ack_table, PositionFileRow *pos, int pos_count){
  Device *device = malloc(sizeof(Device));
  device->num = num;
  device->pid = -1;
  device->board = board;
  device->ack_table = ack_table;
  device->pos = pos;
  device->pos_count = pos_count;
  device->turn_num = 0;
  device->msg_list.next = NULL;

  return device;
}

Device *forkedDevice;
int forkDevice(Device *device){
  pid_t pid = fork();

  //Failed
  if (pid < 0){ return FALSE; }
  //Parent
  if(pid > 0){ 
    device->pid = pid;
    return TRUE; 
  }
  forkedDevice = device;
  device->pid = getpid();
  
	//printf("[ %d ] Sto per fare rundevice..	\n", device->pid);
  runDevice(device);
  exit(0);
}


void initFifoDevice(Device *device){
  char *fifo_path = malloc(sizeof(char)*255);
  //FIFO
  sprintf(fifo_path, DEVICE_QUEUE_PATH, device->pid);
  device->fifo = createFifo(fifo_path); 
}


void runDevice(Device *device){
  initSignalsDevice();
	//Creating local list of messages  //Child (Device)
  initFifoDevice(device);
  
  //printf("\n\n");
  while(TRUE){
    waitTurnBoard(device->board, device->num);

    printf("[ %d ][ %d ]",device->num, device->pid);
    
		// Submit, Recive, move
    submitDevice(device);
    reciveDevice(device);
    moveDevice(device);
    
    printf(".\n");
    device->turn_num++;
    
    endTurnBoard(device->board, device->num);
		
  }

  exit(0);
}

void moveDevice(Device *device){
  printf(" [MOVE] ");
  int x = device->pos->coords[0];
  int y = device->pos->coords[1];
  
  moveFromToBoard(device->board, device->pid, device->x, device->y, x, y);
  printf(" to(%d, %d) ",x,y);
  device->x = x;
  device->y = y;
  
  //PositionFileRow *to_remove = device->pos;
  device->pos = device->pos->next;
  //free(to_remove);
    
}


void reciveDevice(Device *device){
  printf(" [RECEIVE] ");    
  size_t byte_read;
  do{
    printf("reading...");
    MessageList* msg_node = malloc(sizeof(MessageList));
    byte_read = read(device->fifo->fifo_fd, &(msg_node->msg), sizeof(Message));
    if (byte_read != sizeof(Message)){
      //free(msg_node);
      break;
    }

    printf("%d -> msg(%d) ", msg_node->msg.pid_sender, msg_node->msg.message_id);
    msg_node->next= device->msg_list.next;
    device->msg_list.next=msg_node;

    //Aggiorna lista acks
    AckTableRow row_to_add; // = (AckTableRow*)malloc(sizeof(AckTableRow));
    
    row_to_add.pid_sender = msg_node->msg.pid_sender;
    row_to_add.pid_reciever = msg_node->msg.pid_reciever;
    row_to_add.message_id = msg_node->msg.message_id;
    row_to_add.timestamp = time(NULL);
    
    addAckTableRow(device->ack_table, &row_to_add);

  }while(byte_read == sizeof(Message));

}


void submitDevice(Device *device){
    printf(" [SUBMIT] ");
		//TODO: use malloc/free
    pid_t devices_in_range[DEVICE_NUM];
    pid_t devices_received[DEVICE_NUM];

    Message msg;
    MessageList* to_remove = NULL;
    MessageList* prev = NULL;
    MessageList* current = &device->msg_list;
		
    //Loop for every message
    while(current->next != NULL){
      prev = current;
      current = current->next;
      msg = current->msg;

      //Get list of device's pid from Ack Table
      int ack_cnt = searchForMsgInAckTable(device->ack_table, msg.message_id, devices_received); 
      printf("msg(%d):%d ", msg.message_id, ack_cnt);

      
      //Remove message from list if AckTable is full (or empty)
      if ( ack_cnt <= 0 || ack_cnt >= DEVICE_NUM ){
        //REMOVE MSG
        printf(" delete(%d) ", msg.message_id);
        to_remove = current;
        prev->next = current->next;
        current = prev;
        free(to_remove);
      }
      else //Message needs to be submitted
      {
        //Find devices pid in range for given message and distance
        findDeviceBoard(device->board, device->x, device->y, msg.max_distance, devices_in_range);
        //Removes from devices_in_range, the ones that already received the message ( devices_received )
        int submit_cnt = removeDevicesRecived(devices_in_range, devices_received);

        if(submit_cnt > 0){
          printf(" -> ");
        }

        //Send Message to device_in_range:
        //Open other device's FIFO
        for(int j = 0; j < DEVICE_NUM; j++){
          if(devices_in_range[j] != 0){
            pid_t pid_reciever = devices_in_range[j];
            printf(" %d ",(int)pid_reciever);

            msg.pid_sender = device->pid;
            msg.pid_reciever = pid_reciever;
            sendMessage(&msg);
          }
        }
      }
		}
}

int removeDevicesRecived(pid_t *devices_in_range, pid_t *devices_received){
  int cnt = 0;
  for(int i = 0; i < DEVICE_NUM; i++){
    for(int j = 0; j < DEVICE_NUM; j++){
      if(devices_in_range[i] == devices_received[j])
      { devices_in_range[i] = 0; }
      if (devices_in_range[i] != 0)
      { cnt++; }
    }
  }	
  return cnt;
}

void initSignalsDevice(){
		signal(SIGINT, receiveSignalDevice);

		sigset_t mySet;

		//Initialize mySet to contain all signals
		sigfillset(&mySet);

		//Remove SIGINT from mySet
		sigdelset(&mySet, SIGINT);

		//Blocking all signals but SIGTERM
		sigprocmask(SIG_SETMASK, &mySet, 0);
		
}

void receiveSignalDevice(int signal){
  if (signal == SIGINT){
    printf("Ciao sono il device %d, pid: %d\n",forkedDevice->num,getpid());
	  destroyDevice(forkedDevice);
    exit(0);
  }
}


void destroyDevice(Device *device){
  destroyFifo(device->fifo);
  free(device);
}
