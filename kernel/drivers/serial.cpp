#include <minstd/types.hpp>
#include "serial.hpp"

void outb(unsigned short port, unsigned char val) {
  asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

void outw(unsigned short port, unsigned short val) {
  asm volatile ("outw %0, %1" : : "a"(val), "Nd"(port));
}

unsigned char inb(unsigned short port) {
  unsigned char ret;
  asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}

unsigned short inw(unsigned short port) {
  unsigned short ret;
  asm volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}

void init_serial(void) {
  outb(0x3F8 + 1, 0x00); // disable interrupts
  outb(0x3F8 + 3, 0x80); // enable DLAB
  outb(0x3F8 + 0, 0x03); // divisor low (38400 baud)
  outb(0x3F8 + 1, 0x00); // divisor high
  outb(0x3F8 + 3, 0x03); // 8 bits, no parity, one stop bit
  outb(0x3F8 + 2, 0xC7); // enable FIFO
  outb(0x3F8 + 4, 0x0B); // IRQs enabled, RTS/DSR set
}

inline void serial_write(char c) {
  while ((inb(0x3F8 + 5) & 0x20) == 0) {} // wait for transmitter holding register empty
  outb(0x3F8, c);
}

void serial_print(char c) {
  serial_write(c);
}

void serial_print(const char* s) {
  while (*s) {
    if (*s == '\n') serial_write('\r');
    serial_write(*s++);
  }
}

void serial_print(const char* buf, int len) {
  for (int i = 0; i < len; i++) {
    if (buf[i] == '\n') serial_write('\r');
    serial_write(buf[i]);
  }
}

void serial_print(int value) {
  if (value == 0) {
    serial_write('0');
    return;
  }

  if (value < 0) {
    serial_write('-');
    value = -value;
  }

  char buffer[12]; // enough for a 32bit int
  int i = 0;

  while (value > 0) {
    buffer[i++] = '0' + (value % 10);
    value /= 10;
  }

  while (i > 0) serial_write(buffer[--i]);
}
