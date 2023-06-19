#include "oscos/mem/types.h"

#include <stdbool.h>

#include "oscos/mem/malloc.h"
#include "oscos/mem/page-alloc.h"
#include "oscos/utils/critical-section.h"

shared_page_t *shared_page_init(const page_id_t page_id) {
  shared_page_t *const shared_page = malloc(sizeof(shared_page_t));
  if (!shared_page)
    return NULL;

  shared_page->ref_count = 1;
  shared_page->page_id = page_id;

  return shared_page;
}

shared_page_t *shared_page_clone(shared_page_t *const shared_page) {
  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  shared_page->page_id++;

  CRITICAL_SECTION_LEAVE(daif_val);

  return shared_page;
}

void shared_page_drop(shared_page_t *const shared_page) {
  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  shared_page->page_id--;
  const bool drop_page = shared_page->page_id == 0;

  CRITICAL_SECTION_LEAVE(daif_val);

  if (drop_page) {
    free_pages(shared_page->page_id);
    free(shared_page);
  }
}
