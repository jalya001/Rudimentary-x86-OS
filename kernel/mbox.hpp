#pragma once

#include "sync.hpp"

#define MAX_MBOX 12
#define MBUFFER_SIZE 1024
#define MSG_SIZE(m) (sizeof(Message) + m->content_size)

struct Message {
  int content_size;
  char content[0];
} __attribute__((packed));

struct Mbox {
  int owners = 0;
  int messages = 0;
  int head = 0; // first free byte
  int tail = 0; // eldest received
  Lock lock;
  Condition freed_space, arrived_data;
  char buffer[MBUFFER_SIZE];

  int close();
  int stat(int *count, int *space);
  int send(Message *m);
  int recv(Message *m);
};

Mbox* mbox_open(int key);