#include "oscos/framebuffer-dev.h"

#include "oscos/drivers/mailbox.h"
#include "oscos/libc/string.h"
#include "oscos/mem/malloc.h"
#include "oscos/mem/page-alloc.h"
#include "oscos/mem/vm.h"
#include "oscos/uapi/errno.h"
#include "oscos/uapi/unistd.h"
#include "oscos/utils/critical-section.h"
#include "oscos/utils/rb.h"

typedef struct {
  void *framebuffer_base;
  size_t framebuffer_size;
  unsigned int width;
  unsigned int height;
  unsigned int pitch;
  unsigned int isrgb;
} framebuffer_dev_internal_t;

static int _framebuffer_dev_setup_mount(struct device *dev,
                                        struct vnode *vnode);

static int _framebuffer_dev_write(struct file *file, const void *buf,
                                  size_t len);
static int _framebuffer_dev_read(struct file *file, void *buf, size_t len);
static int _framebuffer_dev_open(struct vnode *file_node, struct file **target);
static int _framebuffer_dev_close(struct file *file);
static long _framebuffer_dev_lseek64(struct file *file, long offset,
                                     int whence);
static int _framebuffer_dev_ioctl(struct file *file, unsigned long request,
                                  void *payload);

static int _framebuffer_dev_lookup(struct vnode *dir_node,
                                   struct vnode **target,
                                   const char *component_name);
static int _framebuffer_dev_create(struct vnode *dir_node,
                                   struct vnode **target,
                                   const char *component_name);
static int _framebuffer_dev_mkdir(struct vnode *dir_node, struct vnode **target,
                                  const char *component_name);
static int _framebuffer_dev_mknod(struct vnode *dir_node, struct vnode **target,
                                  const char *component_name,
                                  struct device *device);
static long _framebuffer_dev_get_size(struct vnode *dir_node);

struct device framebuffer_dev = {.name = "framebuffer",
                                 .setup_mount = _framebuffer_dev_setup_mount};

static struct file_operations _framebuffer_dev_file_operations = {
    .write = _framebuffer_dev_write,
    .read = _framebuffer_dev_read,
    .open = _framebuffer_dev_open,
    .close = _framebuffer_dev_close,
    .lseek64 = _framebuffer_dev_lseek64,
    .ioctl = _framebuffer_dev_ioctl};

static struct vnode_operations _framebuffer_dev_vnode_operations = {
    .lookup = _framebuffer_dev_lookup,
    .create = _framebuffer_dev_create,
    .mkdir = _framebuffer_dev_mkdir,
    .mknod = _framebuffer_dev_mknod,
    .get_size = _framebuffer_dev_get_size};

static int _framebuffer_dev_setup_mount(struct device *const dev,
                                        struct vnode *const vnode) {
  (void)dev;

  // Allocate internal data.

  framebuffer_dev_internal_t *const internal =
      malloc(sizeof(framebuffer_dev_internal_t));
  if (!internal)
    return -ENOMEM;

  // Initialize the framebuffer.

  init_framebuffer_result_t init_framebuffer_result =
      mailbox_init_framebuffer();
  if (!init_framebuffer_result.framebuffer_base) {
    free(internal);
    return -EIO;
  }

  // Save vnode data.

  *internal = (framebuffer_dev_internal_t){
      .framebuffer_base = init_framebuffer_result.framebuffer_base,
      .framebuffer_size = init_framebuffer_result.framebuffer_size,
      .width = init_framebuffer_result.width,
      .height = init_framebuffer_result.height,
      .pitch = init_framebuffer_result.pitch,
      .isrgb = init_framebuffer_result.isrgb};

  *vnode = (struct vnode){.mount = vnode->mount,
                          .v_ops = &_framebuffer_dev_vnode_operations,
                          .f_ops = &_framebuffer_dev_file_operations,
                          .internal = internal};

  return 0;
}

static int _framebuffer_dev_write(struct file *const file,
                                  const void *const buf, const size_t len) {
  framebuffer_dev_internal_t *const internal =
      (framebuffer_dev_internal_t *)file->vnode->internal;

  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  if (file->f_pos >= internal->framebuffer_size) {
    CRITICAL_SECTION_LEAVE(daif_val);
    return len == 0 ? 0 : -EFBIG;
  }

  const size_t remaining_len = internal->framebuffer_size - file->f_pos,
               cpy_len = len < remaining_len ? len : remaining_len;

  memcpy((char *)internal->framebuffer_base + file->f_pos, buf, cpy_len);
  file->f_pos += cpy_len;

  CRITICAL_SECTION_LEAVE(daif_val);

  return cpy_len;
}

static int _framebuffer_dev_read(struct file *const file, void *const buf,
                                 const size_t len) {
  (void)file;
  (void)buf;
  (void)len;

  return -EIO;
}

static int _framebuffer_dev_open(struct vnode *const file_node,
                                 struct file **const target) {
  struct file *const file_handle = malloc(sizeof(struct file));
  if (!file_handle)
    return -ENOMEM;

  *file_handle = (struct file){.vnode = file_node,
                               .f_pos = 0,
                               .f_ops = &_framebuffer_dev_file_operations,
                               .flags = 0};
  *target = file_handle;

  return 0;
}

static int _framebuffer_dev_close(struct file *const file) {
  free(file);
  return 0;
}

static long _framebuffer_dev_lseek64(struct file *const file, const long offset,
                                     const int whence) {
  framebuffer_dev_internal_t *const internal =
      (framebuffer_dev_internal_t *)file->vnode->internal;

  if (!(whence == SEEK_SET || whence == SEEK_CUR || whence == SEEK_END))
    return -EINVAL;

  const long f_pos_base = whence == SEEK_SET   ? 0
                          : whence == SEEK_CUR ? file->f_pos
                                               : internal->framebuffer_size,
             new_f_pos = f_pos_base + offset;

  if (new_f_pos < 0) {
    return -EINVAL;
  } else {
    file->f_pos = new_f_pos;
    return 0;
  }
}

static int _framebuffer_dev_ioctl(struct file *const file,
                                  const unsigned long request,
                                  void *const payload) {
  (void)file;

  framebuffer_dev_internal_t *const internal =
      (framebuffer_dev_internal_t *)file->vnode->internal;

  if (request != 0)
    return -ENOTTY;

  framebuffer_info_t *const info = (framebuffer_info_t *)payload;
  *info = (framebuffer_info_t){.width = internal->width,
                               .height = internal->height,
                               .pitch = internal->pitch,
                               .isrgb = internal->isrgb};

  return 0;
}

static int _framebuffer_dev_lookup(struct vnode *const dir_node,
                                   struct vnode **const target,
                                   const char *const component_name) {
  (void)dir_node;
  (void)target;
  (void)component_name;

  return -ENOTDIR;
}

static int _framebuffer_dev_create(struct vnode *const dir_node,
                                   struct vnode **const target,
                                   const char *const component_name) {
  (void)dir_node;
  (void)target;
  (void)component_name;

  return -ENOTDIR;
}

static int _framebuffer_dev_mkdir(struct vnode *const dir_node,
                                  struct vnode **const target,
                                  const char *const component_name) {
  (void)dir_node;
  (void)target;
  (void)component_name;

  return -ENOTDIR;
}

static int _framebuffer_dev_mknod(struct vnode *const dir_node,
                                  struct vnode **const target,
                                  const char *const component_name,
                                  struct device *const device) {
  (void)dir_node;
  (void)target;
  (void)component_name;
  (void)device;

  return -ENOTDIR;
}

static long _framebuffer_dev_get_size(struct vnode *const vnode) {
  const framebuffer_dev_internal_t *const internal = vnode->internal;
  return internal->framebuffer_size;
}
