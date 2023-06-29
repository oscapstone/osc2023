#ifndef OSCOS_MEM_SHARED_PAGE_H
#define OSCOS_MEM_SHARED_PAGE_H

#include "oscos/mem/types.h"

spage_id_t shared_page_alloc(void);
size_t shared_page_getref(page_id_t page) __attribute__((pure));
void shared_page_incref(page_id_t page);
void shared_page_decref(page_id_t page);
spage_id_t shared_page_clone_unshare(page_id_t page);

#endif
