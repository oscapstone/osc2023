#ifndef _SC_ALLOC_H
#define _SC_ALLOC_H

void sc_early_init();
void sc_init();
void *sc_alloc(int size);
int sc_free(void *sc);

#ifdef DEBUG
void sc_test(void);
#endif

#endif