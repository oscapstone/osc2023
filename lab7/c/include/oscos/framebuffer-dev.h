#ifndef OSCOS_FRAMEBUFFER_DEV_H
#define OSCOS_FRAMEBUFFER_DEV_H

#include "oscos/fs/vfs.h"

typedef struct {
  unsigned int width;
  unsigned int height;
  unsigned int pitch;
  unsigned int isrgb;
} framebuffer_info_t;

extern struct device framebuffer_dev;

#endif
