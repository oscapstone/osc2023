#ifndef OSCOS_DRIVERS_SDHOST_H
#define OSCOS_DRIVERS_SDHOST_H

void sd_init(void);
void readblock(int block_idx, void *buf);
void writeblock(int block_idx, void *buf);

#endif
