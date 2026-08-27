#include <syslib.hpp>
#include <sleep.hpp>
#include "sync.hpp"
#include "mbox.hpp"
#include "print.hpp"
#include <utils.hpp>

void kthread2() {
  while (1) {
    sleep(1000);
    kprintf("greetings %d\n", 5);
  }
}
void test_writes() {
  uprintf("Test 1 begin\n");

  while (1) { sleep(500); uprintf("Test 1 testing 111111111\n"); sleep(500); }
}

Lock print_lock;

void locked_print(const char* s) {
  print_lock.acquire();
  kprintf(s);
  print_lock.release();
}

void print(const char* s) {
  kprintf(s);
}

void print_stack(uint32_t bytes) {
  uint32_t* esp;

  asm volatile("movl %%esp, %0" : "=r"(esp));

  for (uint32_t offset = 0; offset <= bytes; offset += 4) {
    uint32_t value = *(uint32_t*)((uint8_t*)esp + offset);

    kprintf("[ESP+0x%x] = 0x%x\n", offset, value);
  }
}

void assert(bool condition) { if (!condition) { critical_kprintf("ERROR: assertion failed\n"); } }




void test_writes_2() {
  uprintf("Test 2 automatic exit\n");
  //while (1) { sleep(500); uprintf("Test 2 testing 2222\n"); sleep(500); }
}

void test_writes_3() {
  uprintf("Test 3 begin\n");

  while (1) { sleep(500); uprintf("Test 3 testing 3\n"); sleep(500); }
}

void test_exiter() {
  uprintf("Exiter: running once, exiting now\n");
  call_exit();
}

void test_after_exit() {
  uprintf("AfterExit: I'm alive — reused a freed stack slot\n");
  while (1) { call_yield(); }
}

void noop_thread() {
  while (1) { call_yield(); }
}

void stress_a() { 
  while (1) { 
    uprintf("A\n"); 
  } 
}

void stress_b() { 
  while (1) { 
    uprintf("B\n"); 
  } 
}

Lock lock;
int lock_counter = 0;

void lock_test_1() {
  locked_print("L1: start\n");

  lock.acquire();
  locked_print("L1: acquired\n");

  lock_counter++;
  locked_print("L1: incremented\n");

  lock_counter++;
  locked_print("L1: incremented\n");

  lock.release();
  locked_print("L1: released\n");
}

void lock_test_2() {
  locked_print("L2: start\n");

  lock.acquire();
  locked_print("L2: acquired\n");

  lock_counter++;
  locked_print("L2: incremented\n");

  lock_counter++;
  locked_print("L2: incremented\n");

  lock.release();
  locked_print("L2: released\n");
}

Lock condition_lock;
Condition condition;
bool ready = false;

void condition_test_waiter() {
  locked_print("C1: start\n");

  condition_lock.acquire();
  locked_print("C1: acquired\n");

  while (!ready) {
    locked_print("C1: waiting\n");
    condition.wait(&condition_lock);
  }

  locked_print("C1: woke\n");
  condition_lock.release();

  locked_print("C1: done\n");

  while (true) {}
}

void condition_test_signaler() {
  locked_print("C2: start\n");

  condition_lock.acquire();
  locked_print("C2: acquired\n");

  ready = true;
  locked_print("C2: signaling\n");

  condition.signal();

  condition_lock.release();
  locked_print("C2: done\n");

  while (true) {}
}

Semaphore semaphore = {1};

void semaphore_test_1() {
  locked_print("S1: start\n");

  semaphore.down();
  locked_print("S1: acquired\n");
  locked_print("S1: working start\n");
  sleep(1000);
  locked_print("S1: working finish\n");

  semaphore.up();
  locked_print("S1: released\n");

  while (true) {}
}

void semaphore_test_2() {
  locked_print("S2: start\n");

  semaphore.down();
  locked_print("S2: acquired\n");
  locked_print("S2: working\n");

  semaphore.up();
  locked_print("S2: released\n");

  while (true) {}
}

Barrier b = {3};

void barrier_test_1() {
  locked_print("B1: before\n");

  b.wait();

  locked_print("B1: after\n");

  while (true) {}
}

void barrier_test_2() {
  locked_print("B2: before\n");

  b.wait();

  locked_print("B2: after\n");

  while (true) {}
}

void barrier_test_3() {
  locked_print("B3: before\n");

  b.wait();

  locked_print("B3: after\n");

  while (true) {}
}

// mbox test sizes need to use bigger data to test full handling 
void test_mbox_1_basic_send_recv() {
  Mbox *mb = mbox_open(0);

  char send_buffer[sizeof(Message) + 5];
  Message *send_msg = (Message *)send_buffer;

  send_msg->content_size = 5;
  send_msg->content[0] = 'H';
  send_msg->content[1] = 'e';
  send_msg->content[2] = 'l';
  send_msg->content[3] = 'l';
  send_msg->content[4] = 'o';

  mb->send(send_msg);

  char recv_buffer[sizeof(Message) + 5];
  Message *recv_msg = (Message *)recv_buffer;

  mb->recv(recv_msg);

  assert(recv_msg->content_size == 5);
  assert(recv_msg->content[0] == 'H');
  assert(recv_msg->content[1] == 'e');
  assert(recv_msg->content[2] == 'l');
  assert(recv_msg->content[3] == 'l');
  assert(recv_msg->content[4] == 'o');

  mb->close();

  print("1 basic send/recv: FINISHED\n");
}

void test_mbox_2_blocking_recv() {
  Mbox *mb = mbox_open(1);

  char recv_buffer[sizeof(Message) + 5];
  Message *msg = (Message *)recv_buffer;

  //print("receiver: waiting...\n");

  mb->recv(msg);

  //print("receiver: received message\n");

  assert(msg->content_size == 5);
  assert(msg->content[0] == 'H');
  assert(msg->content[1] == 'e');
  assert(msg->content[2] == 'l');
  assert(msg->content[3] == 'l');
  assert(msg->content[4] == 'o');

  mb->close();

  print("2 blocking recv: FINISHED\n");
}

void test_mbox_2_blocking_sender() {
  Mbox *mb = mbox_open(1);

  char send_buffer[sizeof(Message) + 5];
  Message *msg = (Message *)send_buffer;

  msg->content_size = 5;
  msg->content[0] = 'H';
  msg->content[1] = 'e';
  msg->content[2] = 'l';
  msg->content[3] = 'l';
  msg->content[4] = 'o';

  //print("sender: sending...\n");

  mb->send(msg);

  //print("sender: sent\n");

  mb->close();
}

void test_mbox_3_multiple_messages() {
  Mbox *mb = mbox_open(2);

  for (int i = 0; i < 10; i++) {
    char send_buffer[sizeof(Message) + 1];
    Message *msg = (Message *)send_buffer;

    msg->content_size = 1;
    msg->content[0] = '0' + i;

    mb->send(msg);
  }

  for (int i = 0; i < 10; i++) {
    char recv_buffer[sizeof(Message) + 1];
    Message *msg = (Message *)recv_buffer;

    mb->recv(msg);

    assert(msg->content_size == 1);
    assert(msg->content[0] == '0' + i);
  }

  mb->close();

  print("3 multiple messages: FINISHED\n");
}

void test_mbox_4_producer1() {
  //kprintf("producer1 begin\n");
  Mbox *mb = mbox_open(3);

  for (int i = 0; i < 100; i++) {
    char send_buffer[sizeof(Message) + 2];
    Message *msg = (Message *)send_buffer;

    msg->content_size = 2;
    msg->content[0] = 'A';
    msg->content[1] = i;

    //kprintf("go send 1 = %c%d\n", msg->content[0],msg->content[1]);
    mb->send(msg);
    //kprintf("send 1 fin = %c%d\n", msg->content[0],msg->content[1]);
  }

  //kprintf("producer1 FINISH\n");
  mb->close();
}

void test_mbox_4_producer2() {
  //kprintf("producer2 begin\n");
  Mbox *mb = mbox_open(3);

  for (int i = 0; i < 100; i++) {
    char send_buffer[sizeof(Message) + 2];
    Message *msg = (Message *)send_buffer;

    msg->content_size = 2;
    msg->content[0] = 'B';
    msg->content[1] = i;

    //kprintf("go send 2 = %c%d\n", msg->content[0],msg->content[1]);
    mb->send(msg);
    //kprintf("send 2 fin = %c%d\n", msg->content[0],msg->content[1]);
  }

  //kprintf("producer2 FINISH\n");
  mb->close();
}

void test_mbox_4_consumer() {
  //kprintf("consumer begin\n");
  Mbox *mb = mbox_open(3);

  for (int i = 0; i < 200; i++) {
    char recv_buffer[sizeof(Message) + 2];
    Message *msg = (Message *)recv_buffer;

    //critical_print("go rec\n");
    mb->recv(msg);
    //kprintf("rec fin = %c%d\n", msg->content[0],msg->content[1]);

    assert(msg->content_size == 2);
    assert(msg->content[0] == 'A' || msg->content[0] == 'B');
    assert(msg->content[1] >= 0 && msg->content[1] < 100);
    //kprintf("consumed %d\n", i);
  }

  mb->close();

  print("4 multiple producers: FINISHED\n");
}

void test_mbox_5_wraparound() {
  Mbox *mb = mbox_open(4);

  for (int round = 0; round < 100; round++) {
    char send_buffer[sizeof(Message) + 1];
    Message *send = (Message *)send_buffer;

    send->content_size = 1;
    send->content[0] = round;

    mb->send(send);

    char recv_buffer[sizeof(Message) + 1];
    Message *recv = (Message *)recv_buffer;

    mb->recv(recv);

    assert(recv->content_size == 1);
    assert(recv->content[0] == round);
  }

  mb->close();

  print("5 wraparound: FINISHED\n");
}

void test_mbox_6_producer() {
  Mbox *mb = mbox_open(5);

  for (uint16_t i = 0; i < 1000; i++) {
    char send_buffer[sizeof(Message) + 4];
    Message *msg = (Message *)send_buffer;

    msg->content_size = 4;
    msg->content[0] = 'H';
    msg->content[1] = 'E';
    ((uint16_t *)msg->content)[1] = i;

    //kprintf("go send 2 = %c%d\n", msg->content[0],msg->content[1]);
    mb->send(msg);
    //kprintf("send 2 fin = %c%d\n", msg->content[0],(uint16_t)msg->content[1]);
  }

  mb->close();
}

void test_mbox_6_consumer() {
  Mbox *mb = mbox_open(5);

  for (uint16_t i = 0; i < 1000; i++) {
    char recv_buffer[sizeof(Message) + 4];
    Message *msg = (Message *)recv_buffer;

    //critical_print("go rec\n");
    mb->recv(msg);
    uint16_t num = ((uint16_t *)msg->content)[1];
    //kprintf("rec fin = %c%c%d\n", msg->content[0],msg->content[1],num);

    assert(msg->content_size == 4);
    assert(msg->content[0] == 'H');
    assert(msg->content[1] == 'E');
    assert(num >= 0 && num < 1000);
    //kprintf("consumed %d\n", i);
  }

  mb->close();

  print("6 more messages than space: FINISHED\n");
}