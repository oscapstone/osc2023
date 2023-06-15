#include "fatfs.h"
#include "heap.h"
#include "mem.h"
#include "str.h"
#include "uart.h"
#include <stddef.h>
#include <stdint.h>

extern struct mount *fsRootMount;
extern struct vnode *fsRoot;

static int BPB;
static uint64_t partition_start;
static uint64_t reserved_blocks;
static uint64_t root_dir;
static uint64_t fat_start;
static uint64_t data_start;
static uint64_t sector_per_fat;
static uint64_t free_sector;
static file_list *dirty_files = NULL;

static void add_list(file_list *xxx, struct file *f) {
  if (dirty_files == NULL) {
    dirty_files = malloc(sizeof(file_list));
    dirty_files->f = f;
    dirty_files->prev = NULL;
    dirty_files->next = NULL;
    //uart_puth(dirty_files);
    return;
  }
  while (dirty_files->next != NULL) {
    dirty_files = dirty_files->next;
  }
  dirty_files->next = malloc(sizeof(file_list));
  dirty_files->next->prev = dirty_files;

  dirty_files = dirty_files->next;
  dirty_files->f = f;
  dirty_files->next = NULL;
  while (dirty_files->prev != NULL)
    dirty_files = dirty_files->prev;
  return;
}

/**************************************************************
 * This function will initialize the FS from Fat (Sd card)
 *
 * @root: The root of this file system.
 *************************************************************/
int fatfs_initFsCpio(struct vnode *root) {
  struct vnode *target = NULL;
  char fat_buf[512] = {0};
  char dir_buf[512] = {0};
  char buf[16] = {0};
  Entry *e = NULL;
  uint64_t size;
  readblock(data_start, dir_buf);
  struct vnode *dir_node = root;
  for (int i = 0; i < 512; i += sizeof(Entry)) {
    e = dir_buf + i;
    memset(buf, 0, sizeof(buf));
    int i, k = 0;
    if (e->name[0] == 0)
      break;
    // Get the file name
    for (i = 0; i < 8; i++) {
      buf[i] = e->name[i];
      if (e->name[i] == ' ') {
        buf[i] = '.';
        break;
      }
    }
    if (buf[i] == '.')
      i++;
    else {
      buf[i] = '.';
      i++;
    }
    for (int j = i; j < i + 3; j++) {
      buf[j] = e->ext[k++];
      if (buf[j] == ' ') {
        buf[j] = '\0';
        break;
      }
    }
    //uart_puts(buf);
    dir_node->v_ops->create(dir_node, &target, buf);
    uint32_t tmp;
    tmp = e->highAddr << 16;
    tmp += e->lowAddr;
    ((FsAttr *)(target->internal))->data = tmp;
    ;
    //uart_puth(tmp);
    ((FsAttr *)(target->internal))->Eof = e->size;
    //uart_puth(e->size);
    //uart_puts("\n");
    target = NULL;
  }

  return 0;
}

/**************************************************************
 * Get the initialize function of the Fatfs
 *************************************************************/
struct filesystem *getFatFs(void) {
  struct filesystem *fs = malloc(sizeof(struct filesystem));
  fs->name = "Fatfs";
  fs->setup_mount = fatfs_init;
  return fs;
}

static void data_init(FsAttr *fs) {
  if (fs->data == NULL) {
    fs->data = (void *)pmalloc(0);
  }
}

/***************************************************************
 * Debug helper function which can dump the Fs tree from root.
 *
 * @cur:	The root of where to start.
 * @depth:	The idention of output.
 **************************************************************/
void fatfs_dump(struct vnode *cur, int depth) {
  if (cur == NULL || cur->internal == NULL)
    return;
  FsAttr *fs = (FsAttr *)(cur->internal);
  for (int i = 0; i < depth; i++) {
    uart_puts(" ");
  }
  if (fs->type == DIRTYPE) {
    uart_puts("dir: ");
    uart_puts(fs->name);
    uart_puts("\n");
    struct vnode **f = (struct vnode **)(fs->dirs);
    for (unsigned i = 0; i < fs->size; i++) {
      fatfs_dump(f[i], depth + 1);
    }
  } else if (fs->type == NORMAL) {
    uart_puts("normal: ");
    uart_puts(fs->name);
    uart_puts("\n");
  } else {
    uart_puts("Unknown file type\n");
  }
  return;
}

/****************************************************************
 * Lookup Implemention.
 ****************************************************************/
int fatfs_lookup(struct vnode *dir, struct vnode **target, const char *name) {
  FsAttr *fs = (FsAttr *)dir->internal;
  if (fs->type != DIRTYPE) {
    uart_puts("Lookup At Normal File\n");
    return 1;
  }
  struct vnode **c = (struct vnode **)fs->dirs;
  for (int i = 0; i < fs->size; i++) {
    struct vnode *ch = c[i];
    FsAttr *cfs = (FsAttr *)ch->internal;
    if (!strcmp(cfs->name, name)) {
      *target = ch;
      return 0;
    }
  }
  *target = NULL;
  return 1;
}

/****************************************************************
 * Create implementation. which will create a new Vnode.
 ***************************************************************/
int fatfs_create(struct vnode *dir, struct vnode **target, const char *name) {
  uart_puts(name);
  FsAttr *fs = (FsAttr *)dir->internal;
  if (fs->type != DIRTYPE) {
    uart_puts("U  should add file only in DIR\n");
    return 1;
  }

  if (fs->dirs == NULL)
    fs->dirs = (struct vnode **)malloc(sizeof(struct vnode *) * 32);
  struct vnode **c = (struct vnode **)fs->dirs;
  *target = (struct vnode *)malloc(sizeof(struct vnode));
  memset(*target, 0, sizeof(struct vnode));
  (*target)->mount = dir->mount;
  (*target)->parent = dir;
  (*target)->v_ops = dir->v_ops;
  (*target)->f_ops = dir->f_ops;
  // Initialize internal data
  (*target)->internal = malloc(sizeof(FsAttr));
  memset((*target)->internal, 0, sizeof(FsAttr));

  FsAttr *cfs = (FsAttr *)(*target)->internal;
  // Initial each field in the interal data
  cfs->name = malloc(16);
  char *t = cfs->name;
  for (int i = 0; i < 16; i++) {
    *t++ = *name++;
  }
  cfs->type = NORMAL;
  cfs->size = 0;
  cfs->data = free_sector;
  free_sector++;
  // Update parent links
  c[(fs->size)++] = (*target);
  return 0;
}

/****************************************************************
 * The function which intialize the  whole file system.
 *
 * @mount: the mount point of the FS.
 ***************************************************************/
int fatfs_init(struct filesystem *fs, struct mount *m) {
  struct vnode *root = m->root;
  char buf[512] = {0};
  root->v_ops =
      (struct vnode_operations *)malloc(sizeof(struct vnode_operations));
  root->v_ops->lookup = fatfs_lookup;
  root->v_ops->create = fatfs_create;
  root->v_ops->mkdir = fatfs_mkdir;

  root->f_ops =
      (struct file_operations *)malloc(sizeof(struct file_operations));
  root->f_ops->open = fatfs_open;
  root->f_ops->write = fatfs_write;
  root->f_ops->read = fatfs_read;
  root->f_ops->close = fatfs_close;

  char *name = NULL;

  // Read the first partition
  sd_init();
  // Read The MBR
  readblock(0, buf);
  // Get the first partition LBA (Little Endian on multiple byte)
  for (int i = 0x1be + 0x0B; i >= 0x1be + 0x08; i--) {
    partition_start <<= 8;
    partition_start += buf[i];
  }
  uart_puth(partition_start);
  uart_puts("\n");

  // Get the fs information of the FAT file system.
  readblock(partition_start, buf);

  // Get reserved blocks in the FAT
  for (int i = 0x0E; i < 0x0E + 1; i++) {
    reserved_blocks <<= 4;
    reserved_blocks += buf[i];
  }
  int fat_nums = buf[16];
  for (int i = 36 + 3; i >= 36; i--) {
    sector_per_fat <<= 8;
    sector_per_fat += buf[i];
  }
  for (int i = 47; i >= 44; i--) {
    root_dir <<= 8;
    root_dir += buf[i];
  }
  uart_puts("root block: ");
  uart_puth(root_dir);

  // Get the next free block in the FAT
  readblock(partition_start + 1, buf);
  for (int i = 495; i >= 492; i--) {
    free_sector <<= 8;
    free_sector += buf[i];
  }
  uart_puts("Next free FAT: ");
  uart_puth(free_sector);
  // Jump to the FAT table
  fat_start = partition_start + reserved_blocks;
  readblock(fat_start, buf);

  data_start = partition_start + reserved_blocks + sector_per_fat * fat_nums;
  // RootDir
  readblock(data_start, buf);
  uart_puts("\nRoot location: ");
  uart_puth(data_start * 512);
  uart_puts("\n");
  for (int i = 0; i < 512; i += sizeof(Entry)) {
    Entry *e = buf + i;
    uart_puts(e->name);
    uart_puts("\n");
  }
  // Check if the data already exist
  if (root->internal == NULL) {
    root->internal = (FsAttr *)malloc(sizeof(FsAttr));
    memset(root->internal, 0, sizeof(FsAttr));
  }

  // is not mount at the root of the whole FS
  if (root->internal != NULL && ((FsAttr *)root->internal)->name != NULL) {
    name = ((FsAttr *)root->internal)->name;
  } else {
    name = malloc(sizeof(char) * 16);
    memset(name, 0, 16);
    *name = '/';
  }

  FsAttr *attr = root->internal;
  attr->name = name;
  attr->type = DIRTYPE;
  attr->size = 0;
  // Use the new dirs
  attr->dirs = (void *)smalloc(32 * sizeof(void *));
  fatfs_initFsCpio(root);
  return 0;
}

/***************************************************************
 * MKDIR
 ***************************************************************/
int fatfs_mkdir(struct vnode *dir, struct vnode **target, const char *name) {
  fatfs_create(dir, target, name);
  FsAttr *attr = (FsAttr *)(*target)->internal;
  attr->type = DIRTYPE;
  return 0;
}

/**************************************************************
 * Open a file and setup properties.
 *************************************************************/
int fatfs_open(struct vnode *v, struct file **target) {
  if (*target == NULL) {
    *target = (struct file *)malloc(sizeof(struct file));
  }
  char *buf = (char *)pmalloc(0);
  (*target)->vnode = v;
  (*target)->f_pos = 0;
  (*target)->f_ops = v->f_ops;
  (*target)->Eof = ((FsAttr *)(v->internal))->Eof;
  readblock(((FsAttr *)(v->internal))->data - 2 + data_start, buf);
  (*target)->data = buf;
  (*target)->dirty = 0;
  return 0;
}

/************************************************************
 * Write the file in Ramfs
 ************************************************************/
int fatfs_write(struct file *f, const void *buf, size_t len) {
  const char *c = (const char *)buf;
  char *data = (char *)f->data;
  f->dirty = 1;
  if (f->data == NULL)
    return 1;
  for (size_t i = f->f_pos; i < len; i++) {
    *(data + (f->f_pos)) = *c++;
    (f->f_pos)++;
  }
  // Update the EOF
  if (f->f_pos > f->Eof)
    f->Eof = f->f_pos;
  return f->f_pos;
}

/************************************************************
 * Read the file in Ramfs
 ************************************************************/
int fatfs_read(struct file *f, void *buf, size_t len) {
  char *c = (char *)buf;
  char *data = (char *)f->data;
  if (f->data == NULL)
    return 0;
  for (size_t i = f->f_pos; i < len; i++) {
    *c++ = *(data + (f->f_pos));
    (f->f_pos)++;
  }
  // IF the pos exceed EOF, just return EOF
  if (f->f_pos > f->Eof)
    return f->Eof;
  return f->f_pos;
}

/***************************************************************
 * Update  the size of the file back to vnode when close
 *************************************************************/
int fatfs_close(struct file *f) {
  ((FsAttr *)(f->vnode->internal))->Eof = f->Eof;

  if (f->dirty != 0)
    add_list(dirty_files, f);

  return 0;
}

void fatfs_sync() {

  // Update the dircetory entry
  struct vnode *target = NULL;
  char fat_buf[512] = {0};
  char dir_buf[512] = {0};
  int k = 0;
  readblock(fat_start, fat_buf);
  Entry *e = NULL;
  uint64_t size;
  int i = 0;
  while (dirty_files != NULL) {
    readblock(data_start + k, dir_buf);
    struct file *f = dirty_files->f;
    char *buf = ((FsAttr *)(f->vnode->internal))->name;
    // Update DIr
    for (i; i < 512; i += sizeof(Entry)) {
      e = (Entry *)(dir_buf + i);
      int ii, k = 0;
      if (dir_buf[i] != 0 && dir_buf[i] != 'A')
        continue;

      // write the file name
      for (ii = 0; ii < 8; ii++) {
        e->name[ii] = buf[ii];
        if (e->name[ii] == '.') {
          e->name[ii] = ' ';
          break;
        }
      }
      for (int j = ii; j < 8; j++)
        e->name[j] = ' ';
      ii++;
      for (int j = 0; j < 3; j++) {
        e->ext[j] = buf[ii++];
      }
      e->highAddr = (((FsAttr *)(f->vnode->internal))->data >> 16) & (65535);
      e->lowAddr = ((FsAttr *)(f->vnode->internal))->data & 65535;
      e->size = f->Eof;
      dir_buf[i + 11] = 0;
      dir_buf[i + 12] = 0;
      i += 32;
      break;
    }
    if (i == 552) {
      k = fat_buf[2];
      root_dir = fat_buf[2] / 512;
      continue;
    }
    // Write back file content
    writeblock(data_start + ((FsAttr *)(f->vnode->internal))->data - 2,
               f->data);
    dirty_files = dirty_files->next;
    writeblock(data_start + k, dir_buf);
  }

  //ramfs_dump(fsRoot, 0);
  // Write back DIR
  char tmp_buf[512] = {0};
  readblock(data_start, tmp_buf);
  writeblock(data_start, dir_buf);
  /*
  for (int i = 0; i < 512; i++) {
    if (tmp_buf[i] == dir_buf[i])
      continue;
    uart_puti(i);
    uart_puth(tmp_buf[i]);
    uart_puts(" ");
    if (i % 16 == 0)
      uart_puts("\n");
  }
  */

  return 0;
}
