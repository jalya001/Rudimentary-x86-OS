#include "mbox.hpp"

Mbox mboxes[MAX_MBOX];

static int space_available(Mbox *mbox) {
  if ((mbox->tail == mbox->head) && (mbox->messages != 0)) return 0; // no space
  if (mbox->tail > mbox->head) return mbox->tail - mbox->head; // wrap around
  return mbox->tail + MBUFFER_SIZE - mbox->head;
}

Mbox* mbox_open(int key) {
  if (key >= MAX_MBOX || key < 0) return (Mbox *)-1;
  mboxes[key].lock.acquire();
  if (mboxes[key].owners == 0 && mboxes[key].messages == 0) {
    // needs count 0 to clear in case consumer has not opened the mbox before producer has finished
    mboxes[key].head = 0;
    mboxes[key].tail = 0;
    mboxes[key].messages = 0;
  }
  mboxes[key].owners++;
  mboxes[key].lock.release();
  return &mboxes[key];
}

int Mbox::close() {
  lock.acquire();
  if (owners == 0) { lock.release(); return -1; }
  owners--;
  lock.release();
  return 1;
}

int Mbox::stat(int *messages_out, int *space_out) {
  lock.acquire();
  *messages_out = messages;
  *space_out = space_available(this);
  lock.release();
  return 1;
}

int Mbox::send(Message *m) { // need a size check to see it doesn't exceed buffer size
  int msize = MSG_SIZE(m);
  lock.acquire();
  while (space_available(this) < msize) freed_space.wait(&lock);

  for (int i = 0; i < msize; i++) buffer[(head + i) % MBUFFER_SIZE] = ((char *)m)[i];
  head = (head + msize) % MBUFFER_SIZE;

  //fd_write(1, "sendon\n");
  messages++;
  //kprintf("%d ", count);
  arrived_data.signal();
  lock.release();
  return 1;
}

inline static void ring_copy_out(Mbox *q, int pos, void *dst, int n) {
  for (int i = 0; i < n; i++) ((char *)dst)[i] = q->buffer[(pos + i) % MBUFFER_SIZE];
}
int Mbox::recv(Message *m) {
  lock.acquire();
  while (messages == 0) arrived_data.wait(&lock);

  ring_copy_out(this, tail, &m->content_size, sizeof(m->content_size));
  // needs to be read first this way because the header could be split due to buffer being circular
  int msize = MSG_SIZE(m);
  ring_copy_out(this, tail, m, msize);
  tail = (tail + msize) % MBUFFER_SIZE;

  //fd_write(1, "recvdon\n");
  messages--;
  freed_space.broadcast();
  lock.release();
  return 1;
}