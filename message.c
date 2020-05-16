#include "message.h"

#include "message.h"

void waitAckQueue(key_t msg_queue_key, int message_id, MessageAckList *msgAckList){ 
  
	int mq_id = msgget(msg_queue_key, S_IRUSR | S_IWUSR);
  printf("waitAckQueue get: %d\n",mq_id);
	
  //Blocking
  int res = msgrcv(mq_id, msgAckList, sizeof( MessageAckList ), (long)message_id, 0);
  printf("waitAckQueue rcv: %d\n",res);

}

//TODO: refactor -> Da inserire il messaggio all'inizio
void saveMessageAck(MessageAckList *msgAckList, char* msg){
  char txt_path[255];
  char txt_msg[512];
  char txt_time[12];

  sprintf(txt_path, CLIENT_OUT_PATH, msgAckList->type); 
  
  //int txtfd = open(txt_path, O_RDWR | O_CREAT | O_EXCL | S_IRUSR, 0777);
  int txtfd = open(txt_path, O_WRONLY | O_CREAT | O_TRUNC | S_IRUSR, 0777);

	//Intestazione
	sprintf(txt_msg, "Messaggio %ld : %s \nLista acknowledgment: \n", msgAckList->type, msg);
	write(txtfd, txt_msg, strlen(txt_msg));

	for(int i = 0; i<5;i++){
    MessageAck msgAck = msgAckList->rows[i];
    struct tm *msgTime = localtime( &msgAck.timestamp );
    strftime(txt_time, 26, MSG_TIME_FORMAT, msgTime);
    sprintf(txt_msg, "%d, %d, %s\n", msgAck.pid_sender, msgAck.pid_reciever, txt_time); 
    write(txtfd, txt_msg, strlen(txt_msg));
  }
	
  close(txtfd);
}

void sendMessage(Message *msg){
  char fifo_path[255];
  sprintf(fifo_path, DEVICE_QUEUE_PATH, (int)msg->pid_reciever); 

  int fd = open(fifo_path, O_WRONLY);
	printf("Ho aperto la FIFO\n");

	write(fd, msg, sizeof(Message));
  printf("Ho scritto sulla FIFO\n");

	close(fd);
	printf("Ho chiuso la FIFO\n");
}

/*
void sendMessage(Message *msg){
  char fifo_path[255];
  sprintf(fifo_path, DEVICE_QUEUE_PATH, (int)msg->pid_reciever); 

  int fd = open(fifo_path, O_WRONLY);
  if (fd == -1){ 
    errExit("Open FIFO failed!");
  }

	if (write(fd, msg, sizeof(Message) == -1)){
		errExit("Write FIFO failed!");
	}

	if (close(fd) == -1){
		errExit("Closing FIFO failed!");
	}
}

void waitAckQueue(key_t msg_queue_key, int message_id, MessageAckList *msgAckList){ 
	int mq_id = msgget(msg_queue_key, S_IRUSR | S_IWUSR);
  printf("waitAckQueue get: %d\n",mq_id);
	
  //Blocking
  printf("Waiting for Ack messages\t");
  int res = msgrcv(mq_id, msgAckList, sizeof( MessageAckList ), (long)message_id, 0);
  printf("[ %s ]\n",(res == -1 ? "ERROR":"OK"));
  //printf("waitAckQueue rcv: %d\n",res);

}

//TODO: refactor -> Da inserire il messaggio all'inizio
void saveMessageAck(MessageAckList *msgAckList, char* msg){
  char txt_path[255];
  char txt_msg[512];
  char txt_time[12];

  sprintf(txt_path, CLIENT_OUT_PATH, msgAckList->type); 

  int txtfd = open(txt_path, O_WRONLY | O_CREAT | O_TRUNC | S_IRUSR, 0777);

	//Intestazione
	sprintf(txt_msg, "Messaggio %ld : %s \nLista acknowledgment: \n", msgAckList->type, msg);
	write(txtfd, txt_msg, strlen(txt_msg));

	for(int i = 0; i<5;i++){
    MessageAck msgAck = msgAckList->rows[i];
    struct tm *msgTime = localtime( &msgAck.timestamp );
    strftime(txt_time, 26, MSG_TIME_FORMAT, msgTime);
    sprintf(txt_msg, "%d, %d, %s\n", msgAck.pid_sender, msgAck.pid_reciever, txt_time); 
    write(txtfd, txt_msg, strlen(txt_msg));
  }
	
  close(txtfd);
}
*/