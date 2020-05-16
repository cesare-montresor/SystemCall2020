/// @file client.c
/// @brief Contiene l'implementazione del client.

#include "defines.h"
#include "ack_table.h"
#include "message.h"

#include <sys/stat.h> 
#include <sys/msg.h> 

int main(int argc, char * argv[]) {
  
  //Read msg_queue's key
  key_t msg_queue_key = atoi(argv[1]);
  if(msg_queue_key <= 0){
    printf("The message_queue_key must be greater than zero!\n");
    exit(1);
  }
		
  Message msg;
  msg.pid_sender = getpid();

  printf("Pid reciever\n");
  scanf(" %d", &msg.pid_reciever);

  printf("Message ID\n");
  scanf(" %d", &msg.message_id);

	//Se con spazio, prende anche Max_distance
  printf("Message\n");
  scanf(" %s", msg.message);

  printf("Max distance\n");
  scanf(" %lf", &msg.max_distance);

  //Send the message
  sendMessage(&msg);
  
  //Wait for ack message
  MessageAckList msgAckList;
	waitAckQueue(msg_queue_key, msg.message_id, &msgAckList);

	//Create a txt file, with the info contained in ackMsg
  saveMessageAck(&msgAckList, msg.message);

  return 0;
}


