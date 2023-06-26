#ifndef __SD_H
#define __SD_H

#include "type.h"

#define SECTOR_SIZE 512

void sd_init();
void writeblock(int block_idx, void* buf);
void readblock(int block_idx, void* buf);

#endif