#ifndef OSCOS_UTILS_RB_H
#define OSCOS_UTILS_RB_H

#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>

// typedef enum { RB_NC_BLACK, RB_NC_RED } rb_node_colour_t;

typedef struct rb_node_t {
  // rb_node_colour_t colour;
  struct rb_node_t *children[2] /*, **parent */;
  alignas(16) unsigned char payload[];
} rb_node_t;

rb_node_t *rb_clone(const rb_node_t *root, size_t size,
                    bool (*cloner)(void *dst, const void *src),
                    void (*deleter)(void *payload));

const void *rb_search(const rb_node_t *root, const void *restrict key,
                      int (*compar)(const void *, const void *, void *),
                      void *arg);

const void *rb_predecessor(const rb_node_t *root, const void *restrict key,
                           int (*compar)(const void *, const void *, void *),
                           void *arg);

const void *rb_successor(const rb_node_t *root, const void *restrict key,
                         int (*compar)(const void *, const void *, void *),
                         void *arg);

bool rb_insert(rb_node_t **root, size_t size, const void *restrict item,
               int (*compar)(const void *, const void *, void *), void *arg);

void rb_delete(rb_node_t **root, const void *restrict key,
               int (*compar)(const void *, const void *, void *), void *arg);

void rb_drop(rb_node_t *root, void (*deleter)(void *payload));

#endif
