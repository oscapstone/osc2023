#include "oscos/fs/vfs.h"

#include <stdint.h>

#include "oscos/libc/string.h"
#include "oscos/mem/malloc.h"
#include "oscos/uapi/errno.h"
#include "oscos/utils/critical-section.h"
#include "oscos/utils/rb.h"

typedef struct {
  struct vnode *mountpoint;
  struct mount *mount;
} mount_entry_t;

struct mount rootfs;

static rb_node_t *_filesystems = NULL;
static rb_node_t *_mounts_by_mountpoint = NULL, *_mounts_by_root = NULL;

static int _vfs_cmp_filesystems_by_name(const struct filesystem *const fs1,
                                        const struct filesystem *const fs2,
                                        void *const _arg) {
  (void)_arg;

  return strcmp(fs1->name, fs2->name);
}

static int _vfs_cmp_name_and_filesystem(const char *const name,
                                        const struct filesystem *const fs,
                                        void *const _arg) {
  (void)_arg;

  return strcmp(name, fs->name);
}

static int _vfs_cmp_mounts_by_mountpoint(const mount_entry_t *const m1,
                                         const mount_entry_t *const m2,
                                         void *const _arg) {
  (void)_arg;

  if (m1->mountpoint < m2->mountpoint)
    return -1;
  if (m1->mountpoint > m2->mountpoint)
    return 1;
  return 0;
}

static int _vfs_cmp_mountpoint_and_mount(const struct vnode *const mountpoint,
                                         const mount_entry_t *const mount,
                                         void *const _arg) {
  (void)_arg;

  if (mountpoint < mount->mountpoint)
    return -1;
  if (mountpoint > mount->mountpoint)
    return 1;
  return 0;
}

static int _vfs_cmp_mounts_by_root(const mount_entry_t *const m1,
                                   const mount_entry_t *const m2,
                                   void *const _arg) {
  (void)_arg;

  if (m1->mount->root < m2->mount->root)
    return -1;
  if (m1->mount->root > m2->mount->root)
    return 1;
  return 0;
}

static int _vfs_cmp_root_and_mount(const struct vnode *const root,
                                   const mount_entry_t *const mount,
                                   void *const _arg) {
  (void)_arg;

  if (root < mount->mount->root)
    return -1;
  if (root > mount->mount->root)
    return 1;
  return 0;
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

static int _vfs_lookup_step(struct vnode *const curr_vnode,
                            struct vnode **const target,
                            const char *const component_name) {
  const int result =
      curr_vnode->v_ops->lookup(curr_vnode, target, component_name);
  if (result < 0)
    return result;

  // If the new node is the mountpoint of a mounted file system, jump to the
  // filesystem root.

  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  const mount_entry_t *const entry =
      rb_search(_mounts_by_mountpoint, *target,
                (int (*)(const void *, const void *,
                         void *))_vfs_cmp_mountpoint_and_mount,
                NULL);

  CRITICAL_SECTION_LEAVE(daif_val);

  if (entry) {
    *target = entry->mount->root;
  }

  return 0;
}

static int
_vfs_lookup_sans_last_level(const char *const pathname,
                            struct vnode **const target,
                            const char **const last_pathname_component) {
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

    const int result =
        _vfs_lookup_step(curr_vnode, target, curr_pathname_component_copy);
    free(curr_pathname_component_copy);
    if (result < 0)
      return result;
    curr_vnode = *target;
  }

  *target = curr_vnode;
  *last_pathname_component = curr_pathname_component;
  return 0;
}

int vfs_open(const char *const pathname, const int flags,
             struct file **const target) {
  const char *last_pathname_component;
  struct vnode *parent_vnode;
  const int lookup_result = _vfs_lookup_sans_last_level(
      pathname, &parent_vnode, &last_pathname_component);
  if (lookup_result < 0)
    return lookup_result;

  struct vnode *curr_vnode;
  const int result = parent_vnode->v_ops->lookup(parent_vnode, &curr_vnode,
                                                 last_pathname_component);
  if (result == -ENOENT && flags & O_CREAT) {
    const int result = parent_vnode->v_ops->create(parent_vnode, &curr_vnode,
                                                   last_pathname_component);
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

int vfs_mkdir(const char *const pathname) {
  const char *last_pathname_component;
  struct vnode *parent_vnode;
  const int lookup_result = _vfs_lookup_sans_last_level(
      pathname, &parent_vnode, &last_pathname_component);
  if (lookup_result < 0)
    return lookup_result;

  struct vnode *target;
  return parent_vnode->v_ops->mkdir(parent_vnode, &target,
                                    last_pathname_component);
}

int vfs_mount(const char *const target, const char *const filesystem) {
  struct vnode *mountpoint;
  const int result = vfs_lookup(target, &mountpoint);
  if (result < 0)
    return result;

  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  struct filesystem *fs = (struct filesystem *)rb_search(
      _filesystems, filesystem,
      (int (*)(const void *, const void *, void *))_vfs_cmp_name_and_filesystem,
      NULL);

  CRITICAL_SECTION_LEAVE(daif_val);

  if (!fs)
    return -ENODEV;

  struct mount *const mount = malloc(sizeof(struct mount));
  if (!mount)
    return -ENOMEM;

  fs->setup_mount(fs, mount);

  const mount_entry_t entry = {.mountpoint = mountpoint, .mount = mount};

  CRITICAL_SECTION_ENTER(daif_val);

  rb_insert(&_mounts_by_mountpoint, sizeof(mount_entry_t), &entry,
            (int (*)(const void *, const void *,
                     void *))_vfs_cmp_mounts_by_mountpoint,
            NULL);

  CRITICAL_SECTION_LEAVE(daif_val);

  CRITICAL_SECTION_ENTER(daif_val);

  rb_insert(
      &_mounts_by_root, sizeof(mount_entry_t), &entry,
      (int (*)(const void *, const void *, void *))_vfs_cmp_mounts_by_root,
      NULL);

  CRITICAL_SECTION_LEAVE(daif_val);

  return 0;
}

int vfs_lookup(const char *const pathname, struct vnode **const target) {
  const char *last_pathname_component;
  struct vnode *parent_vnode;
  const int lookup_result = _vfs_lookup_sans_last_level(
      pathname, &parent_vnode, &last_pathname_component);
  if (lookup_result < 0)
    return lookup_result;

  return _vfs_lookup_step(parent_vnode, target, last_pathname_component);
}
