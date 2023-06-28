#include "oscos/fs/vfs.h"

#include <stdint.h>

#include "oscos/libc/string.h"
#include "oscos/mem/malloc.h"
#include "oscos/uapi/errno.h"
#include "oscos/utils/critical-section.h"
#include "oscos/utils/rb.h"

struct mount rootfs;

static rb_node_t *_filesystems = NULL;

static int _vfs_cmp_filesystems_by_name(const struct filesystem *const fs1,
                                        const struct filesystem *const fs2,
                                        void *const _arg) {
  (void)_arg;

  return strcmp(fs1->name, fs2->name);
}

int register_filesystem(struct filesystem *const fs) {
  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  rb_insert(
      &_filesystems, sizeof(struct filesystem), fs,
      (int (*)(const void *, const void *, void *))_vfs_cmp_filesystems_by_name,
      NULL);

  CRITICAL_SECTION_LEAVE(daif_val);

  return 0;
}

int vfs_open(const char *const pathname, const int flags,
             struct file **const target) {
  struct vnode *curr_vnode = rootfs.root;
  const char *curr_pathname_component = pathname + 1;
  for (const char *curr_pathname_component_end;
       (curr_pathname_component_end = strchr(curr_pathname_component, '/'));
       curr_pathname_component = curr_pathname_component_end + 1) {
    const size_t curr_pathname_component_len =
        curr_pathname_component_end - curr_pathname_component;
    char *const curr_pathname_component_copy =
        strndup(curr_pathname_component, curr_pathname_component_len);
    if (!curr_pathname_component_copy)
      return -ENOMEM;

    const int result = curr_vnode->v_ops->lookup(curr_vnode, &(*target)->vnode,
                                                 curr_pathname_component_copy);
    free(curr_pathname_component_copy);
    if (result < 0)
      return result;
    curr_vnode = (*target)->vnode;
  }

  struct vnode *const parent_vnode = curr_vnode;

  const int result = parent_vnode->v_ops->lookup(parent_vnode, &curr_vnode,
                                                 curr_pathname_component);
  if (result == -ENOENT && flags & O_CREAT) {
    const int result = parent_vnode->v_ops->create(parent_vnode, &curr_vnode,
                                                   curr_pathname_component);
    if (result < 0)
      return result;
  } else if (result < 0) {
    return result;
  }
  return curr_vnode->f_ops->open(curr_vnode, target);
}

int vfs_close(struct file *const file) { return file->f_ops->close(file); }

int vfs_write(struct file *const file, const void *const buf,
              const size_t len) {
  return file->f_ops->write(file, buf, len);
}

int vfs_read(struct file *const file, void *const buf, const size_t len) {
  return file->f_ops->read(file, buf, len);
}
