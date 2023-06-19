#include "oscos/utils/rb.h"

#include "oscos/libc/string.h"
#include "oscos/mem/malloc.h"

// TODO: Properly implement a red-black tree.

const void *rb_search(const rb_node_t *const root,
                      const void *const restrict key,
                      int (*const compar)(const void *, const void *, void *),
                      void *const arg) {
  const rb_node_t *curr = root;
  while (curr) {
    const int compar_result = compar(key, curr->payload, arg);
    if (compar_result == 0)
      break;
    curr = curr->children[compar_result > 0];
  }

  return curr ? curr->payload : NULL;
}

bool rb_insert(rb_node_t **const root, const size_t size,
               const void *const restrict item,
               int (*const compar)(const void *, const void *, void *),
               void *const arg) {
  rb_node_t **curr = root;
  while (*curr) {
    const int compar_result = compar(item, (*curr)->payload, arg);
    if (compar_result == 0)
      break;
    curr = &(*curr)->children[compar_result > 0];
  }

  if (!*curr) {
    *curr = malloc(sizeof(rb_node_t) + size);
    if (!*curr)
      return false;

    (*curr)->children[0] = (*curr)->children[1] = NULL;
  }

  memcpy((*curr)->payload, item, size);

  return true;
}

static rb_node_t **_rb_minimum(rb_node_t **const node) {
  rb_node_t **curr = node;
  while ((*curr)->children[0]) {
    curr = &(*curr)->children[0];
  }
  return curr;
}

void rb_delete(rb_node_t **const root, const void *const restrict key,
               int (*const compar)(const void *, const void *, void *),
               void *const arg) {
  rb_node_t **curr = root;
  while (*curr) {
    const int compar_result = compar(key, (*curr)->payload, arg);
    if (compar_result == 0)
      break;
    curr = &(*curr)->children[compar_result > 0];
  }

  rb_node_t *const curr_node = *curr;
  if (!curr_node->children[0]) {
    *curr = curr_node->children[1];
  } else if (!curr_node->children[1]) {
    *curr = curr_node->children[0];
  } else {
    rb_node_t **const min_right = _rb_minimum(&curr_node->children[1]),
                      *const min_right_node = *min_right;

    *min_right = min_right_node->children[1];
    *curr = min_right_node;
    min_right_node->children[0] = curr_node->children[0];
    min_right_node->children[1] = curr_node->children[1];
  }

  free(curr_node);
}
