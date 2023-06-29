#ifndef OSCOS_VM_PAGE_TABLE_H
#define OSCOS_VM_PAGE_TABLE_H

#include <stdbool.h>

typedef struct {
  unsigned _reserved0 : (50 - 48 + 1);
  unsigned ignored : (58 - 51 + 1);
  bool pxntable : 1;
  bool uxntable : 1;
  unsigned aptable : 2;
  unsigned _reserved1 : 1;
} table_descriptor_upper_t;

typedef struct {
  unsigned _reserved0 : (51 - 48 + 1);
  bool contiguous : 1;
  bool pxn : 1;
  bool uxn : 1;
  unsigned ignored : (63 - 55 + 1);
} block_page_descriptor_upper_t;

typedef struct {
  unsigned attr_indx : 3;
  bool _reserved0 : 1;
  unsigned ap : 2;
  unsigned sh : 2;
  bool af : 1;
  bool ng : 1;
} block_page_descriptor_lower_t;

typedef struct {
  bool b0 : 1;
  bool b1 : 1;
  unsigned lower : (11 - 2 + 1);
  unsigned long long addr : (47 - 12 + 1);
  unsigned upper : (63 - 48 + 1);
} page_table_entry_t;

typedef page_table_entry_t page_table_t[512];

#endif
