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
static rb_node_t *_devices = NULL;
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

static int _vfs_cmp_devices_by_name(const struct device *const dev1,
                                    const struct device *const dev2,
                                    void *const _arg) {
  (void)_arg;

  return strcmp(dev1->name, dev2->name);
}

static int _vfs_cmp_name_and_device(const char *const name,
                                    const struct device *const dev,
                                    void *const _arg) {
  (void)_arg;

  return strcmp(name, dev->name);
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

int register_device(struct device *const dev) {
  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  rb_insert(
      &_devices, sizeof(struct device), dev,
      (int (*)(const void *, const void *, void *))_vfs_cmp_devices_by_name,
      NULL);

  CRITICAL_SECTION_LEAVE(daif_val);

  return 0;
}

static int _vfs_lookup_step(struct vnode *curr_vnode,
                            struct vnode **const target,
                            const char *const component_name) {
  if (strcmp(component_name, "..") == 0 &&
      curr_vnode == curr_vnode->mount->root) {
    if (curr_vnode == rootfs.root) {
      *target = curr_vnode;
      return 0;
    } else {
      uint64_t daif_val;
      CRITICAL_SECTION_ENTER(daif_val);

      const mount_entry_t *const entry = rb_search(
          _mounts_by_root, curr_vnode,
          (int (*)(const void *, const void *, void *))_vfs_cmp_root_and_mount,
          NULL);

      CRITICAL_SECTION_LEAVE(daif_val);

      curr_vnode = entry->mountpoint;
    }
  }

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

static int _vfs_lookup_sans_last_level_relative(
    struct vnode *const cwd, const char *const pathname,
    struct vnode **const target, const char **const last_pathname_component) {
  struct vnode *curr_vnode = cwd;
  const char *curr_pathname_component = pathname;
  if (*curr_pathname_component == '/') {
    curr_vnode = rootfs.root;
    curr_pathname_component += 1;
  }

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
  return vfs_open_relative(rootfs.root, pathname, flags, target);
}

int vfs_open_relative(struct vnode *const cwd, const char *const pathname,
                      const int flags, struct file **const target) {
  const char *last_pathname_component;
  struct vnode *parent_vnode;
  const int lookup_result = _vfs_lookup_sans_last_level_relative(
      cwd, pathname, &parent_vnode, &last_pathname_component);
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

long vfs_lseek64(struct file *const file, const long offset, const int whence) {
  return file->f_ops->lseek64(file, offset, whence);
}

int vfs_ioctl(struct file *const file, const unsigned long request,
              void *const payload) {
  return file->f_ops->ioctl(file, request, payload);
}

int vfs_mkdir(const char *const pathname) {
  return vfs_mkdir_relative(rootfs.root, pathname);
}

int vfs_mkdir_relative(struct vnode *const cwd, const char *const pathname) {
  const char *last_pathname_component;
  struct vnode *parent_vnode;
  const int lookup_result = _vfs_lookup_sans_last_level_relative(
      cwd, pathname, &parent_vnode, &last_pathname_component);
  if (lookup_result < 0)
    return lookup_result;

  struct vnode *target;
  return parent_vnode->v_ops->mkdir(parent_vnode, &target,
                                    last_pathname_component);
}

int vfs_mount(const char *const target, const char *const filesystem) {
  return vfs_mount_relative(rootfs.root, target, filesystem);
}

int vfs_mount_relative(struct vnode *const cwd, const char *const target,
                       const char *const filesystem) {
  struct vnode *mountpoint;
  const int result = vfs_lookup_relative(cwd, target, &mountpoint);
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

  const int setup_mount_result = fs->setup_mount(fs, mount);
  if (setup_mount_result < 0) {
    free(mount);
    return setup_mount_result;
  }

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
  return vfs_lookup_relative(rootfs.root, pathname, target);
}

int vfs_lookup_relative(struct vnode *const cwd, const char *const pathname,
                        struct vnode **const target) {
  const char *last_pathname_component;
  struct vnode *parent_vnode;
  const int lookup_result = _vfs_lookup_sans_last_level_relative(
      cwd, pathname, &parent_vnode, &last_pathname_component);
  if (lookup_result < 0)
    return lookup_result;

  if (*last_pathname_component == '\0') {
    *target = parent_vnode;
    return 0;
  } else {
    return _vfs_lookup_step(parent_vnode, target, last_pathname_component);
  }
}

int vfs_mknod(const char *target, const char *device) {
  // Lookup the pathname.

  struct vnode *mountpoint;
  const char *last_pathname_component;
  const int result = _vfs_lookup_sans_last_level_relative(
      rootfs.root, target, &mountpoint, &last_pathname_component);
  if (result < 0)
    return result;

  // Find the device struct.

  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  struct device *dev = (struct device *)rb_search(
      _devices, device,
      (int (*)(const void *, const void *, void *))_vfs_cmp_name_and_device,
      NULL);

  CRITICAL_SECTION_LEAVE(daif_val);

  if (!dev)
    return -ENODEV;

  struct vnode *target_vnode;
  return mountpoint->v_ops->mknod(mountpoint, &target_vnode,
                                  last_pathname_component, dev);
}

shared_file_t *shared_file_new(struct file *const file) {
  shared_file_t *const shared_file = malloc(sizeof(shared_file_t));
  if (!shared_file)
    return NULL;

  *shared_file = (shared_file_t){.file = file, .refcnt = 1};
  return shared_file;
}

shared_file_t *shared_file_clone(shared_file_t *const shared_file) {
  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  shared_file->refcnt++;

  CRITICAL_SECTION_LEAVE(daif_val);

  return shared_file;
}

void shared_file_drop(shared_file_t *const shared_file) {
  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  shared_file->refcnt--;
  const bool drop = shared_file->refcnt == 0;

  CRITICAL_SECTION_LEAVE(daif_val);

  if (drop) {
    vfs_close(shared_file->file);
    free(shared_file);
  }
}
