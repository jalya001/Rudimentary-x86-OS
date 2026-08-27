#pragma once

void init_serial(void);
void outb(unsigned short port, unsigned char val);
void outw(unsigned short port, unsigned short val);
unsigned char inb(unsigned short port);
unsigned short inw(unsigned short port);
void serial_print(const char* s);
void serial_print(char c);
void serial_print(const char* buf, int len);
void serial_print(int value);
