#include "sync.hpp"
#include "kernel.hpp"
#include "entry.hpp"
#include "scheduler.hpp"

inline static void lock_acquire_helper(Lock *l) {
  while (l->status == LOCKED) block(&l->queue); // needs to be a loop because mulltiple could be unblocked at once
  l->status = LOCKED;
}

inline static void lock_release_helper(Lock *l) {
  l->status = UNLOCKED;
  if (l->queue != nullptr) unblock(&l->queue);
}

void Lock::acquire() {
  enter_critical();
  lock_acquire_helper(this);
  leave_critical();
}

void Lock::release() {
  enter_critical();
  lock_release_helper(this);
  leave_critical();
}

void Condition::wait(Lock *l) {
  enter_critical();
  lock_release_helper(l);
  block(&queue);
  lock_acquire_helper(l);
  leave_critical();
}

void Condition::signal() {
  enter_critical();
  if (queue != nullptr) unblock(&queue);
  leave_critical();
}

void Condition::broadcast() {
  enter_critical();
  while (queue != nullptr) unblock(&queue);
  leave_critical();
}

void Semaphore::up() {
  enter_critical();
  if (queue == nullptr) {
    slots++;
  } else {
    unblock(&queue);
  }
  leave_critical();
}

void Semaphore::down() {
  enter_critical();
  if (slots > 0) {
    slots--;
  } else {
    block(&queue);
  }
  leave_critical();
}

void Barrier::wait() {
  enter_critical();
  counter--;
  if (counter > 0) {
    block(&queue);
  } else {
    while (queue != nullptr) unblock(&queue), counter++;
    counter++;
  }
  leave_critical();
}