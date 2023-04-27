#ifndef	_BOOT_H
#define	_BOOT_H

#define BUFFER_SIZE 256

extern void delay ( unsigned long);
extern void put32 ( unsigned long, unsigned int );
extern unsigned int get32 ( unsigned long );

int strcmp(char *, char *);
int strncmp(const char *, const char *, unsigned int);
int strtol(const char *, char **, int);
void strcpy(char *, const char *);
void *memcpy(void *, const void *, unsigned int);
unsigned int strlen(const char *);
unsigned int htonl(unsigned int);

#endif  /*_BOOT_H */
