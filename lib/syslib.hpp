#pragma once

void call_yield(void);
void call_exit(void);
int getpid(void);
/*
int mbox_open(int key);
int mbox_close(int q);
int mbox_stat(int q, int *count, int *space);
int mbox_recv(int q, char *m);
int mbox_send(int q, char *m);
*/
int getchar(int *c);

void call_write(const char *msg);

int readdir(unsigned char *buf);
void loadproc(int location, int size);

int fs_open(const char *filename, int mode);
int fs_close(int fd);
int fs_read(int fd, char *buffer, int size);
int fs_write(int fd, char *buffer, int size);
int fs_lseek(int fd, int offset, int whence);
int fs_link(char *linkname, char *filename);
int fs_unlink(char *linkname);
int fs_stat(int fd, char *buffer);
