#ifndef FATFS_H
#define FATFS_H

#include "mem.h"
#include "str.h"
#include "uart.h"
#include "vfs.h"

#include <stdint.h>

typedef struct {
  char *name;
  int type;
  int size;
  size_t Eof;
  void *dirs;
  void *data;
} FsAttr;

// For the directory
typedef struct {
  char name[8];	// SFN care tof the first byte
  char ext[3];	// Extension pad with " "
  uint8_t attr;	// File attribute Normal -> 0
  uint8_t attr2; // default: 0 
  uint8_t createTimeMs;	// ms of the createTime
  uint16_t createTime;	// 15:13 -> Hours, 10:5 -> Min, 4:0 -> seconds
  uint16_t createTimeDate;	// 15:13 -> Year, 10:5 -> Mon, 4:0 -> Day
  uint16_t ownerID;
  uint16_t highAddr;	// High two bytes of first cluster 
  uint16_t modifiedTime;
  uint16_t modifiedDate;
  uint16_t lowAddr;	// Low two bytes of first cluster 
  uint32_t size;	// Total size of file
} Entry;

int fatfs_init(struct filesystem *fs, struct mount *m);

struct filesystem *getRamFs();

// This function is to initial from cpio archive
int fatfs_initFsCpio(struct vnode *);

int fatfs_lookup(struct vnode *dir_node, struct vnode **trget, const char *);
int fatfs_create(struct vnode *dir_node, struct vnode **trget, const char *);
int fatfs_mkdir(struct vnode *dir_node, struct vnode **trget, const char *);

int fatfs_read(struct file *f, void *buf, size_t len);
int fatfs_write(struct file *f, const void *buf, size_t len);
int fatfs_open(struct vnode *, struct file **target);
int fatfs_close(struct file *f);


#endif
