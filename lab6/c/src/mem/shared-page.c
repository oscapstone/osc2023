#include "oscos/mem/shared-page.h"

#include "oscos/libc/string.h"
#include "oscos/mem/page-alloc.h"
#include "oscos/mem/vm.h"
#include "oscos/utils/critical-section.h"
#include "oscos/utils/rb.h"

static rb_node_t *_page_refcnts = NULL;

typedef struct {
  page_id_t page_id;
  size_t refcnt;
} page_refcnt_entry_t;

static int
_cmp_page_refcnt_entry_by_page_id(const page_refcnt_entry_t *const e1,
                                  const page_refcnt_entry_t *const e2,
                                  void *_arg) {
  (void)_arg;

  if (e1->page_id < e2->page_id)
    return -1;
  if (e1->page_id > e2->page_id)
    return 1;
  return 0;
}

static int
_cmp_page_id_and_page_refcnt_entry(const page_id_t *const page,
                                   const page_refcnt_entry_t *const entry,
                                   void *const _arg) {
  (void)_arg;

  if (*page < entry->page_id)
    return -1;
  if (*page > entry->page_id)
    return 1;
  return 0;
}

spage_id_t shared_page_alloc(void) {
  spage_id_t result = alloc_pages(0);
  if (result < 0)
    return result;

  const page_refcnt_entry_t new_entry = {.page_id = result, .refcnt = 1};

  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  rb_insert(&_page_refcnts, sizeof(page_refcnt_entry_t), &new_entry,
            (int (*)(const void *, const void *,
                     void *))_cmp_page_refcnt_entry_by_page_id,
            NULL);

  CRITICAL_SECTION_LEAVE(daif_val);

  return result;
}

size_t shared_page_getref(const page_id_t page) {
  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  const page_refcnt_entry_t *const entry =
      rb_search(_page_refcnts, &page,
                (int (*)(const void *, const void *,
                         void *))_cmp_page_id_and_page_refcnt_entry,
                NULL);
  const size_t result = entry->refcnt;

  CRITICAL_SECTION_LEAVE(daif_val);

  return result;
}

void shared_page_incref(const page_id_t page) {
  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  // It's safe to cast away const here, since every entry _page_refcnts points
  // to is not const. Also, incrementing the reference count doesn't invalidate
  // the BST invariant.
  page_refcnt_entry_t *const entry = (page_refcnt_entry_t *)rb_search(
      _page_refcnts, &page,
      (int (*)(const void *, const void *,
               void *))_cmp_page_id_and_page_refcnt_entry,
      NULL);

  // This function is sometimes called on a non-shared page; more specifically,
  // linearly-mapped pages.
  if (entry) {
    entry->refcnt++;
  }

  CRITICAL_SECTION_LEAVE(daif_val);
}

void shared_page_decref(const page_id_t page) {
  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  // It's safe to cast away const here, since every entry _page_refcnts points
  // to is not const. Also, incrementing the reference count doesn't invalidate
  // the BST invariant.
  page_refcnt_entry_t *const entry = (page_refcnt_entry_t *)rb_search(
      _page_refcnts, &page,
      (int (*)(const void *, const void *,
               void *))_cmp_page_id_and_page_refcnt_entry,
      NULL);
  entry->refcnt--;

  if (entry->refcnt == 0) {
    rb_delete(&_page_refcnts, &page,
              (int (*)(const void *, const void *,
                       void *))_cmp_page_id_and_page_refcnt_entry,
              NULL);
    free_pages(page);
  }

  CRITICAL_SECTION_LEAVE(daif_val);
}

spage_id_t shared_page_clone_unshare(const page_id_t page) {
  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  if (shared_page_getref(page) == 1) { // No need to clone.
    CRITICAL_SECTION_LEAVE(daif_val);
    return page;
  }

  shared_page_decref(page);

  spage_id_t new_page_id = shared_page_alloc();
  if (new_page_id < 0) {
    CRITICAL_SECTION_LEAVE(daif_val);
    return new_page_id;
  }

  memcpy(pa_to_kernel_va(page_id_to_pa(new_page_id)),
         pa_to_kernel_va(page_id_to_pa(page)), 1 << PAGE_ORDER);

  CRITICAL_SECTION_LEAVE(daif_val);
  return new_page_id;
}
