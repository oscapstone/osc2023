#ifndef _SDHOST_H
#define _SDHOST_H

#define SD_BLOCK_SIZE 512
#define SECTOR_SIZE 512

void readblock(int block_idx, void *buf);
void writeblock(int block_idx, void *buf);
void sd_init();

#endif