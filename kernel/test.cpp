#include <syslib.hpp>
#include <sleep.hpp>
#include "sync.hpp"
#include "drivers/serial.hpp"

void test_writes() {
  call_write("Test 1 begin\n");

  while (1) { sleep(500); call_write("Test 1 testing 111111111\n"); sleep(500); }
  //while (1) { call_yield(); }
}

void test_writes_2() {
  call_write("Test 2 begin\n");

  while (1) { sleep(500); call_write("Test 2 testing 2222\n"); sleep(500); }
}

void test_writes_3() {
  call_write("Test 3 begin\n");

  while (1) { sleep(500); call_write("Test 3 testing 3\n"); sleep(500); }
}

void test_exiter() {
  call_write("Exiter: running once, exiting now\n");
  call_exit();
}

void test_after_exit() {
  call_write("AfterExit: I'm alive — reused a freed stack slot\n");
  while (1) { call_yield(); }
}

void noop_thread() {
  while (1) { call_yield(); }
}

void stress_a() { 
  while (1) { 
    call_write("A\n"); 
  } 
}

void stress_b() { 
  while (1) { 
    call_write("B\n"); 
  } 
}

Lock print_lock;

void locked_print(const char* s) {
  print_lock.acquire();
  fd_write(1, s);
  print_lock.release();
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

  while (true) {}
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

  while (true) {}
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