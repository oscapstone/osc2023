#include "oscos/fs/initramfs.h"

#include "oscos/initrd.h"
#include "oscos/libc/string.h"
#include "oscos/mem/malloc.h"
#include "oscos/uapi/errno.h"
#include "oscos/uapi/stdio.h"
#include "oscos/utils/critical-section.h"
#include "oscos/utils/rb.h"

typedef struct {
  const char *component_name;
  struct vnode *vnode;
} initramfs_child_vnode_entry_t;

typedef struct {
  struct vnode *parent;
  const cpio_newc_entry_t *entry;
  rb_node_t *child_vnodes; // Created lazily.
} initramfs_internal_t;

static int _initramfs_setup_mount(struct filesystem *fs, struct mount *mount);

static int _initramfs_write(struct file *file, const void *buf, size_t len);
static int _initramfs_read(struct file *file, void *buf, size_t len);
static int _initramfs_open(struct vnode *file_node, struct file **target);
static int _initramfs_close(struct file *file);
static long _initramfs_lseek64(struct file *file, long offset, int whence);
static int _initramfs_ioctl(struct file *file, unsigned long request,
                            void *payload);

static int _initramfs_lookup(struct vnode *dir_node, struct vnode **target,
                             const char *component_name);
static int _initramfs_create(struct vnode *dir_node, struct vnode **target,
                             const char *component_name);
static int _initramfs_mkdir(struct vnode *dir_node, struct vnode **target,
                            const char *component_name);
int _initramfs_mknod(struct vnode *dir_node, struct vnode **target,
                     const char *component_name, struct device *device);

struct filesystem initramfs = {.name = "initramfs",
                               .setup_mount = _initramfs_setup_mount};

static struct file_operations _initramfs_file_operations = {
    .write = _initramfs_write,
    .read = _initramfs_read,
    .open = _initramfs_open,
    .close = _initramfs_close,
    .lseek64 = _initramfs_lseek64,
    .ioctl = _initramfs_ioctl};

static struct vnode_operations _initramfs_vnode_operations = {
    .lookup = _initramfs_lookup,
    .create = _initramfs_create,
    .mkdir = _initramfs_mkdir,
    .mknod = _initramfs_mknod};

static int _initramfs_cmp_child_vnode_entries_by_component_name(
    const initramfs_child_vnode_entry_t *const e1,
    const initramfs_child_vnode_entry_t *const e2, void *const _arg) {
  (void)_arg;

  return strcmp(e1->component_name, e2->component_name);
}

static int _initramfs_cmp_component_name_and_child_vnode_entry(
    const char *component_name,
    const initramfs_child_vnode_entry_t *const entry, void *const _arg) {
  (void)_arg;

  return strcmp(component_name, entry->component_name);
}

static struct vnode *
_initramfs_create_vnode(struct mount *const mount, struct vnode *const parent,
                        const cpio_newc_entry_t *const entry) {
  struct vnode *const result = malloc(sizeof(struct vnode));
  if (!result)
    return NULL;

  initramfs_internal_t *const internal = malloc(sizeof(initramfs_internal_t));
  if (!internal) {
    free(result);
    return NULL;
  }

  *internal = (initramfs_internal_t){
      .parent = parent, .entry = entry, .child_vnodes = NULL};
  *result = (struct vnode){.mount = mount,
                           .v_ops = &_initramfs_vnode_operations,
                           .f_ops = &_initramfs_file_operations,
                           .internal = internal};
  return result;
}

static int _initramfs_setup_mount(struct filesystem *const fs,
                                  struct mount *const mount) {
  struct vnode *const root_vnode = _initramfs_create_vnode(mount, NULL, NULL);
  if (!root_vnode)
    return -ENOMEM;

  *mount = (struct mount){.fs = fs, .root = root_vnode};

  return 0;
}

static int _initramfs_write(struct file *const file, const void *const buf,
                            const size_t len) {
  (void)file;
  (void)buf;
  (void)len;

  return -EROFS;
}

static int _initramfs_read(struct file *const file, void *const buf,
                           const size_t len) {
  initramfs_internal_t *const internal =
      (initramfs_internal_t *)file->vnode->internal;
  const cpio_newc_entry_t *const entry = internal->entry;

  const uint32_t mode = CPIO_NEWC_HEADER_VALUE(entry, mode);
  const uint32_t file_type = mode & CPIO_NEWC_MODE_FILE_TYPE_MASK;
  if (file_type == CPIO_NEWC_MODE_FILE_TYPE_REG) {
    // No-op.
  } else if (file_type == CPIO_NEWC_MODE_FILE_TYPE_DIR) {
    return -EISDIR;
  } else if (file_type == CPIO_NEWC_MODE_FILE_TYPE_LNK) {
    return -ELOOP;
  } else { // Unknown file type.
    return -EIO;
  }

  const size_t file_size = CPIO_NEWC_FILESIZE(entry);

  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  const size_t remaining_len =
                   file->f_pos >= file_size ? 0 : file_size - file->f_pos,
               cpy_len = len < remaining_len ? len : remaining_len;

  memcpy(buf, CPIO_NEWC_FILE_DATA(entry) + file->f_pos, cpy_len);
  file->f_pos += cpy_len;

  CRITICAL_SECTION_LEAVE(daif_val);

  return cpy_len;
}

static int _initramfs_open(struct vnode *const file_node,
                           struct file **const target) {
  initramfs_internal_t *const internal =
      (initramfs_internal_t *)file_node->internal;
  const cpio_newc_entry_t *const entry = internal->entry;

  const uint32_t mode = CPIO_NEWC_HEADER_VALUE(entry, mode);
  const uint32_t file_type = mode & CPIO_NEWC_MODE_FILE_TYPE_MASK;
  if (file_type == CPIO_NEWC_MODE_FILE_TYPE_REG) {
    // No-op.
  } else if (file_type == CPIO_NEWC_MODE_FILE_TYPE_DIR) {
    return -EISDIR;
  } else if (file_type == CPIO_NEWC_MODE_FILE_TYPE_LNK) {
    return -ELOOP;
  } else { // Unknown file type.
    return -EIO;
  }

  struct file *const file_handle = malloc(sizeof(struct file));
  if (!file_handle)
    return -ENOMEM;

  *file_handle = (struct file){.vnode = file_node,
                               .f_pos = 0,
                               .f_ops = &_initramfs_file_operations,
                               .flags = 0};
  *target = file_handle;

  return 0;
}

static int _initramfs_close(struct file *const file) {
  free(file);
  return 0;
}

static long _initramfs_lseek64(struct file *const file, const long offset,
                               const int whence) {
  initramfs_internal_t *const internal =
      (initramfs_internal_t *)file->vnode->internal;
  const cpio_newc_entry_t *const entry = internal->entry;

  const uint32_t mode = CPIO_NEWC_HEADER_VALUE(entry, mode);
  const uint32_t file_type = mode & CPIO_NEWC_MODE_FILE_TYPE_MASK;
  if (file_type == CPIO_NEWC_MODE_FILE_TYPE_REG) {
    // No-op.
  } else if (file_type == CPIO_NEWC_MODE_FILE_TYPE_DIR) {
    return -EISDIR;
  } else if (file_type == CPIO_NEWC_MODE_FILE_TYPE_LNK) {
    return -ELOOP;
  } else { // Unknown file type.
    return -EIO;
  }

  if (!(whence == SEEK_SET || whence == SEEK_CUR || whence == SEEK_END))
    return -EINVAL;

  const long f_pos_base = whence == SEEK_SET   ? 0
                          : whence == SEEK_CUR ? file->f_pos
                                               : CPIO_NEWC_FILESIZE(entry),
             new_f_pos = f_pos_base + offset;

  if (new_f_pos < 0) {
    return -EINVAL;
  } else {
    file->f_pos = new_f_pos;
    return 0;
  }
}

static int _initramfs_ioctl(struct file *const file,
                            const unsigned long request, void *const payload) {
  (void)file;
  (void)request;
  (void)payload;

  return -ENOTTY;
}

static int _initramfs_lookup(struct vnode *const dir_node,
                             struct vnode **const target,
                             const char *const component_name) {
  initramfs_internal_t *const internal =
      (initramfs_internal_t *)dir_node->internal;
  const cpio_newc_entry_t *const entry = internal->entry;

  if (dir_node != dir_node->mount->root) { // The vnode of the root directory
                                           // doesn't have an entry.
    const uint32_t mode = CPIO_NEWC_HEADER_VALUE(entry, mode);
    const uint32_t file_type = mode & CPIO_NEWC_MODE_FILE_TYPE_MASK;
    if (file_type == CPIO_NEWC_MODE_FILE_TYPE_REG) {
      return -ENOTDIR;
    } else if (file_type == CPIO_NEWC_MODE_FILE_TYPE_DIR) {
      // No-op.
    } else if (file_type == CPIO_NEWC_MODE_FILE_TYPE_LNK) {
      return -ELOOP;
    } else { // Unknown file type.
      return -EIO;
    }
  }

  if (strcmp(component_name, ".") == 0) {
    *target = dir_node;
    return 0;
  } else if (strcmp(component_name, "..") == 0) {
    *target = internal->parent;
    return 0;
  }

  // Check if the vnode has been created before.

  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  const initramfs_child_vnode_entry_t *const child_vnode_entry = rb_search(
      internal->child_vnodes, component_name,
      (int (*)(const void *, const void *,
               void *))_initramfs_cmp_component_name_and_child_vnode_entry,
      NULL);

  CRITICAL_SECTION_LEAVE(daif_val);

  if (child_vnode_entry) {
    *target = child_vnode_entry->vnode;
    return 0;
  }

  // A vnode has not been created before. Check if the file/directory exists.

  size_t dirname_len;
  const cpio_newc_entry_t *child_initrd_entry;

  if (dir_node != dir_node->mount->root) {
    dirname_len = strlen(CPIO_NEWC_PATHNAME(entry));
    char *const filename_buf =
        malloc(dirname_len + 1 + strlen(component_name) + 1);
    if (!filename_buf)
      return -ENOMEM;
    memcpy(filename_buf, CPIO_NEWC_PATHNAME(entry), dirname_len);
    filename_buf[dirname_len] = '/';
    strcpy(filename_buf + dirname_len + 1, component_name);

    child_initrd_entry = initrd_find_entry_by_pathname(filename_buf);
    free(filename_buf);
  } else {
    child_initrd_entry = initrd_find_entry_by_pathname(component_name);
  }
  if (!child_initrd_entry) {
    return -ENOENT;
  }

  // Create a new vnode.

  struct vnode *const vnode =
      _initramfs_create_vnode(dir_node->mount, dir_node, child_initrd_entry);
  if (!vnode)
    return -ENOMEM;

  const initramfs_child_vnode_entry_t new_child_vnode_entry = {
      .component_name =
          CPIO_NEWC_PATHNAME(child_initrd_entry) +
          (dir_node != dir_node->mount->root ? dirname_len + 1 : 0),
      .vnode = vnode};

  CRITICAL_SECTION_ENTER(daif_val);

  rb_insert(&internal->child_vnodes, sizeof(initramfs_child_vnode_entry_t),
            &new_child_vnode_entry,
            (int (*)(const void *, const void *, void *))
                _initramfs_cmp_child_vnode_entries_by_component_name,
            NULL);

  CRITICAL_SECTION_LEAVE(daif_val);

  *target = vnode;

  return 0;
}

static int _initramfs_create(struct vnode *const dir_node,
                             struct vnode **const target,
                             const char *const component_name) {
  (void)dir_node;
  (void)target;
  (void)component_name;

  return -EROFS;
}

static int _initramfs_mkdir(struct vnode *const dir_node,
                            struct vnode **const target,
                            const char *const component_name) {
  (void)dir_node;
  (void)target;
  (void)component_name;

  return -EROFS;
}

int _initramfs_mknod(struct vnode *const dir_node, struct vnode **const target,
                     const char *const component_name,
                     struct device *const device) {
  (void)dir_node;
  (void)target;
  (void)component_name;
  (void)device;

  return -EROFS;
}
