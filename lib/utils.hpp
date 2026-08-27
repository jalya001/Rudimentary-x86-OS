/* Provides prints and most functions which would otherwise be found in the standard library. */

void clear_screen(int minx, int miny, int maxx, int maxy);
void scroll(int minx, int miny, int maxx, int maxy);

void screen_print(const char *in, ...);
void serial_print(const char *in, ...);
void uprintf(const char *in, ...);

void reverse(char *s);
int strlen(const char *s);

void strcpy(char *dest, char *source);
int strncmp(const char *s, const char *t, size_t n);
int strcmp(const char *s, const char *t);
char *strncpy(char *dest, const char *src, int len);
int strlcpy(char *dest, const char *src, int size);

int same_string(char *s1, char *s2); // calls strcmp

void buffer_copy(const char *source, char *destin, int size);
void buffer_zero(char *a, int size);

void srand(uint32_t seed);
int rand(void);

int atoi(const char *s);
void dtoa(double dbl, char *s, int s_len);
void itoa(uint32_t n, char *s);
void itohex(uint32_t n, char *s);

uint32_t ntohl(uint32_t data);
uint32_t htonl(uint32_t data);
uint16_t ntohs(uint16_t data);
uint16_t htons(uint16_t data);