
#pragma once

#ifndef _MESSAGE_HH
#define _MESSAGE_HH

#include "defines.h"
#include "utils.h"

struct Message{
  pid_t pid_sender;
  pid_t pid_reciever;
  int message_id;
  char message[256];
  double max_distance;
};
typedef struct Message Message;

struct MessageList{
  Message msg;
  struct MessageList *next;
};
typedef struct MessageList MessageList;

// Client <- Ack Manager
struct MessageAck{
  pid_t pid_sender;
  pid_t pid_reciever;
  time_t timestamp;
};
typedef struct MessageAck MessageAck;

struct MessageAckList{
  long type;
  MessageAck rows[5];
};
typedef struct MessageAckList MessageAckList;


void waitAckQueue(key_t msg_queue_key, int pid, MessageAckList *msgAckList);
void saveMessageAck(MessageAckList *msgAckList, char* msg);

void sendMessage(Message *msg);

#endif