#pragma once

void test_writes();
void test_writes_2();
void test_writes_3();
void test_exiter();
void test_after_exit();
void noop_thread();
void stress_a();
void stress_b();

void lock_test_1();
void lock_test_2();
void condition_test_waiter();
void condition_test_signaler();
void semaphore_test_1();
void semaphore_test_2();
void barrier_test_1();
void barrier_test_2();
void barrier_test_3();

void test_mbox_1_basic_send_recv();
void test_mbox_2_blocking_recv();
void test_mbox_2_blocking_sender();
void test_mbox_3_multiple_messages();
void test_mbox_4_producer1();
void test_mbox_4_producer2();
void test_mbox_4_consumer();
void test_mbox_5_wraparound();
void test_mbox_6_producer();
void test_mbox_6_consumer();