/// @file client.c
/// @brief Contiene l'implementazione del client.

#include "defines.h"
#include "ack_table.h"
#include "message.h"

#include <sys/stat.h> 
#include <sys/msg.h>  

#define ARG_BIN 0
#define ARG_KEY 1
#define ARG_PID 2
#define ARG_ID 3
#define ARG_MSG 4
#define ARG_DIST 5


int main(int argc, char * argv[]) { 
  
  if(argc < 2){
    printf("\nUsage:\n $ %s <msg_queue_key> [pid_reciever] [message_id] [message] [max_distance]\n",argv[ARG_BIN]);
    printf("\nExamples\n");
    printf(" $ %s 42\n",argv[ARG_BIN]);
    printf(" $ %s 42 1327 1 \"ciao mondo\" 10\n",argv[ARG_BIN]);
    printf(" $ %s 42 1347\n",argv[ARG_BIN]);
    
    printf("\n");
    exit(1);
  }

  //Read msg_queue's key
  key_t msg_queue_key = atoi(argv[ARG_KEY]);
  //printf("%d",msg_queue_key);
  if(msg_queue_key <= 0){
    printf("The message_queue_key must be greater than zero!\n");
    exit(1);
  }


  Message msg;
  msg.pid_sender = getpid();

  if (argc <= ARG_PID){
    printf("Pid reciever\n");
    scanf(" %d", &msg.pid_reciever); 
  }else
  { 
    msg.pid_reciever = atoi(argv[ARG_PID]);
  }

  if (argc <= ARG_ID){
    printf("Message ID\n");
    scanf(" %d", &msg.message_id);
  }else
  { 
    msg.message_id = atoi(argv[ARG_ID]);
  }
  
  if (argc <= ARG_MSG){
    printf("Message\n");
    scanf(" %s", msg.message);
  }else
  { 
    strcpy(msg.message, "argv[ARG_MSG]");
  }

  if (argc <= ARG_DIST){
    printf("Max distance\n");
    scanf(" %lf", &msg.max_distance);
  }else
  { 
    msg.max_distance = atoi(argv[ARG_DIST]);
  }
  
  //Send the message
  printf("\nSending message\n");
  printf("PID Sender:\t\t%d\n", msg.pid_sender);
  printf("PID Reciever:\t%d\n", msg.pid_reciever);
  printf("Message ID:\t\t%d\n", msg.message_id);
  printf("Max distance:\t%1.lf\n", msg.max_distance);
  printf("Message: \t\t%s\n", msg.message);
  printf("\n");
  
  sendMessage(&msg);
  
  printf("\nWait for ACK message\n");
  //Wait for ack message
  MessageAckList msgAckList;
	waitAckQueue(msg_queue_key, msg.message_id, &msgAckList);

	//Create a txt file, with the info contained in ackMsg
  saveMessageAck(&msgAckList, msg.message);

  return 0;
}


