#include "oscos/console-dev.h"

#include "oscos/console-suspend.h"
#include "oscos/libc/string.h"
#include "oscos/mem/malloc.h"
#include "oscos/mem/page-alloc.h"
#include "oscos/mem/vm.h"
#include "oscos/uapi/errno.h"
#include "oscos/uapi/stdio.h"
#include "oscos/utils/critical-section.h"
#include "oscos/utils/rb.h"

static int _console_dev_setup_mount(struct device *dev, struct vnode *vnode);

static int _console_dev_write(struct file *file, const void *buf, size_t len);
static int _console_dev_read(struct file *file, void *buf, size_t len);
static int _console_dev_open(struct vnode *file_node, struct file **target);
static int _console_dev_close(struct file *file);
static long _console_dev_lseek64(struct file *file, long offset, int whence);

static int _console_dev_lookup(struct vnode *dir_node, struct vnode **target,
                               const char *component_name);
static int _console_dev_create(struct vnode *dir_node, struct vnode **target,
                               const char *component_name);
static int _console_dev_mkdir(struct vnode *dir_node, struct vnode **target,
                              const char *component_name);
int _console_mknod(struct vnode *dir_node, struct vnode **target,
                   const char *component_name, struct device *device);

struct device console_dev = {.name = "console",
                             .setup_mount = _console_dev_setup_mount};

static struct file_operations _console_dev_file_operations = {
    .write = _console_dev_write,
    .read = _console_dev_read,
    .open = _console_dev_open,
    .close = _console_dev_close,
    .lseek64 = _console_dev_lseek64};

static struct vnode_operations _console_dev_vnode_operations = {
    .lookup = _console_dev_lookup,
    .create = _console_dev_create,
    .mkdir = _console_dev_mkdir,
    .mknod = _console_mknod};

static int _console_dev_setup_mount(struct device *const dev,
                                    struct vnode *const vnode) {
  (void)dev;

  *vnode = (struct vnode){.mount = vnode->mount,
                          .v_ops = &_console_dev_vnode_operations,
                          .f_ops = &_console_dev_file_operations,
                          .internal = NULL};
  return 0;
}

static int _console_dev_write(struct file *const file, const void *const buf,
                              const size_t len) {
  (void)file;

  return console_write_suspend(buf, len);
}

static int _console_dev_read(struct file *const file, void *const buf,
                             const size_t len) {
  (void)file;

  return console_read_suspend(buf, len);
}

static int _console_dev_open(struct vnode *const file_node,
                             struct file **const target) {
  struct file *const file_handle = malloc(sizeof(struct file));
  if (!file_handle)
    return -ENOMEM;

  *file_handle = (struct file){.vnode = file_node,
                               .f_pos = 0,
                               .f_ops = &_console_dev_file_operations,
                               .flags = 0};
  *target = file_handle;

  return 0;
}

static int _console_dev_close(struct file *const file) {
  free(file);
  return 0;
}

static long _console_dev_lseek64(struct file *const file, const long offset,
                                 const int whence) {
  (void)file;
  (void)offset;
  (void)whence;

  return -ESPIPE;
}

static int _console_dev_lookup(struct vnode *const dir_node,
                               struct vnode **const target,
                               const char *const component_name) {
  (void)dir_node;
  (void)target;
  (void)component_name;

  return -ENOTDIR;
}

static int _console_dev_create(struct vnode *const dir_node,
                               struct vnode **const target,
                               const char *const component_name) {
  (void)dir_node;
  (void)target;
  (void)component_name;

  return -ENOTDIR;
}

static int _console_dev_mkdir(struct vnode *const dir_node,
                              struct vnode **const target,
                              const char *const component_name) {
  (void)dir_node;
  (void)target;
  (void)component_name;

  return -ENOTDIR;
}

int _console_mknod(struct vnode *const dir_node, struct vnode **const target,
                   const char *const component_name,
                   struct device *const device) {
  (void)dir_node;
  (void)target;
  (void)component_name;
  (void)device;

  return -ENOTDIR;
}
