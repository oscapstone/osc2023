#ifndef	BOOT_H
#define	BOOT_H

extern void delay(unsigned long);
extern void put32(unsigned long, unsigned int);
extern unsigned int get32(unsigned long);
extern void branch_to_address(void*);

#endif /* BOOT_H */