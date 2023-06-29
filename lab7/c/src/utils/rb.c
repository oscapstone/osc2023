#include "oscos/utils/rb.h"

#include "oscos/libc/string.h"
#include "oscos/mem/malloc.h"

// TODO: Properly implement a red-black tree.

rb_node_t *rb_clone(const rb_node_t *const root, const size_t size,
                    bool (*const cloner)(void *dst, const void *src),
                    void (*const deleter)(void *payload)) {
  if (!root)
    return NULL;

  rb_node_t *const new_root = malloc(sizeof(rb_node_t) + size);
  if (!new_root)
    return NULL;

  for (size_t i = 0; i < 2; i++) {
    new_root->children[i] = rb_clone(root->children[i], size, cloner, deleter);
    if (root->children[i] && !new_root->children[i]) { // Out of memory.
      for (size_t j = 0; j < i; j++) {
        rb_drop(new_root->children[j], deleter);
      }
      return NULL;
    }
  }

  if (cloner) {
    const bool clone_successful = cloner(new_root->payload, root->payload);
    if (!clone_successful) {
      for (size_t i = 0; i < 2; i++) {
        rb_drop(new_root->children[i], deleter);
      }
      return NULL;
    }
  } else {
    memcpy(new_root->payload, root->payload, size);
  }

  return new_root;
}

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

const void *
rb_predecessor(const rb_node_t *const root, const void *const restrict key,
               int (*const compar)(const void *, const void *, void *),
               void *const arg) {
  const rb_node_t *curr = root, *result = NULL;
  while (curr) {
    const int compar_result = compar(key, curr->payload, arg);

    if (compar_result >= 0) {
      result = curr;
    }

    if (compar_result == 0) {
      break;
    }

    curr = curr->children[compar_result > 0];
  }

  return result ? result->payload : NULL;
}

const void *
rb_successor(const rb_node_t *const root, const void *const restrict key,
             int (*const compar)(const void *, const void *, void *),
             void *const arg) {
  const rb_node_t *curr = root, *result = NULL;
  while (curr) {
    const int compar_result = compar(key, curr->payload, arg);

    if (compar_result <= 0) {
      result = curr;
    }

    if (compar_result == 0) {
      break;
    }

    curr = curr->children[compar_result > 0];
  }

  return result ? result->payload : NULL;
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

void rb_drop(rb_node_t *root, void (*deleter)(void *payload)) {
  if (!root)
    return;

  if (deleter) {
    deleter(root->payload);
  }

  for (size_t i = 0; i < 2; i++) {
    rb_drop(root->children[i], deleter);
  }
  free(root);
}
