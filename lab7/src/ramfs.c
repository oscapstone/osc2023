#include "ramfs.h"
#include "heap.h"
#include "initrd.h"
#include "mem.h"
#include "str.h"
#include "uart.h"
#include <stddef.h>
#include <stdint.h>

extern struct mount *fsRootMount;
extern struct vnode *fsRoot;

/**************************************************************
 * This function will initialize the FS from CPIO archive
 *
 * @root: The root of this file system.
 *************************************************************/
int ramfs_initFsCpio(struct vnode *root) {
  void *f = initrd_getLo(); // get the location of cpio
  struct vnode *target = NULL;
  char buf[16] = {0};
  uint64_t size;
  while (1) {
    char *fname = initrd_getName(f);
    char *fdata = initrd_getData(f);
    int fsize = initrd_getSize(f);
    int fmode = initrd_getMode(f);
    // uart_puts(fname);
    //  End
    if (strcmp(fname, "TRAILER!!!") == 0)
      break;
    struct vnode *dir_node = root;
    while (fname != NULL) {
      memset(buf, 0, 16);
      fname = getFileName(buf, fname);

      // The Directory must be the directory type
      ((FsAttr *)(dir_node->internal))->type = DIRTYPE;
      if (*buf != 0) {
        // Find if the dir is exist
        dir_node->v_ops->lookup(dir_node, &target, buf);
        // If not exist, just create it
        if (target == NULL)
          dir_node->v_ops->create(dir_node, &target, buf);
      }
      dir_node = target;
      ((FsAttr *)(target->internal))->data = fdata;
      ((FsAttr *)(target->internal))->Eof = fsize;
      target = NULL;
    }
    f = initrd_jumpNext(f);
  }
  return 0;
}

/**************************************************************
 * Get the initialize function of the Ramfs
 *************************************************************/
struct filesystem *getRamFs(void) {
  struct filesystem *fs = malloc(sizeof(struct filesystem));
  fs->name = "Ramfs";
  fs->setup_mount = ramfs_init;
  return fs;
}

void data_init(FsAttr *fs) {
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
void ramfs_dump(struct vnode *cur, int depth) {
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
      ramfs_dump(f[i], depth + 1);
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
int ramfs_lookup(struct vnode *dir, struct vnode **target, const char *name) {
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
int ramfs_create(struct vnode *dir, struct vnode **target, const char *name) {
  FsAttr *fs = (FsAttr *)dir->internal;
  if (fs->type != DIRTYPE) {
    uart_puts("U  should add file only in DIR\n");
    return 1;
  }

  if (fs->dirs == NULL)
    fs->dirs = (struct vnode **)malloc(sizeof(struct vnode *) * 16);
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
  cfs->data = pmalloc(0);
  // Update parent links
  c[(fs->size)++] = (*target);
  return 0;
}

/****************************************************************
 * The function which intialize the  whole file system.
 *
 * @mount: the mount point of the FS.
 ***************************************************************/
int ramfs_init(struct filesystem *fs, struct mount *m) {
  struct vnode *root = m->root;
  root->v_ops =
      (struct vnode_operations *)malloc(sizeof(struct vnode_operations));
  root->v_ops->lookup = ramfs_lookup;
  root->v_ops->create = ramfs_create;
  root->v_ops->mkdir = ramfs_mkdir;

  root->f_ops =
      (struct file_operations *)malloc(sizeof(struct file_operations));
  root->f_ops->open = ramfs_open;
  root->f_ops->write = ramfs_write;
  root->f_ops->read = ramfs_read;
  root->f_ops->close = ramfs_close;

  char *name = NULL;

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
  // Preserve the created dirs
  if (attr->dirs == NULL) {
    attr->dirs = (void *)smalloc(8 * sizeof(void *));
  }
  return 0;
}

/***************************************************************
 * MKDIR
 ***************************************************************/
int ramfs_mkdir(struct vnode *dir, struct vnode **target, const char *name) {
  ramfs_create(dir, target, name);
  FsAttr *attr = (FsAttr *)(*target)->internal;
  attr->type = DIRTYPE;
  return 0;
}

/**************************************************************
 * Open a file and setup properties.
 *************************************************************/
int ramfs_open(struct vnode *v, struct file **target) {
  if (*target == NULL) {
    *target = (struct file *)malloc(sizeof(struct file));
  }
  (*target)->vnode = v;
  (*target)->f_pos = 0;
  (*target)->f_ops = v->f_ops;
  (*target)->Eof = ((FsAttr *)(v->internal))->Eof;
  (*target)->data = ((FsAttr *)(v->internal))->data;
  return 0;
}

/************************************************************
 * Write the file in Ramfs
 ************************************************************/
int ramfs_write(struct file *f, const void *buf, size_t len) {
  const char *c = (const char *)buf;
  char *data = (char *)f->data;
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
int ramfs_read(struct file *f, void *buf, size_t len) {
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
int ramfs_close(struct file *f) {
  ((FsAttr *)(f->vnode->internal))->Eof = f->Eof;
  return 0;
}
