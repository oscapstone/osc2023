#include "oscos/fs/tmpfs.h"

#include "oscos/libc/string.h"
#include "oscos/mem/malloc.h"
#include "oscos/mem/page-alloc.h"
#include "oscos/mem/vm.h"
#include "oscos/uapi/errno.h"
#include "oscos/uapi/stdio.h"
#include "oscos/utils/critical-section.h"
#include "oscos/utils/rb.h"

#define MAX_FILE_SZ (1 << PAGE_ORDER)

typedef enum { TYPE_DIR, TYPE_FILE } tmpfs_internal_type_t;

typedef struct {
  struct vnode *parent;
  rb_node_t *contents;
} tmpfs_internal_dir_data_t;

typedef struct {
  unsigned char *contents;
  size_t size;
} tmpfs_internal_file_data_t;

typedef struct {
  tmpfs_internal_type_t type;
  union {
    tmpfs_internal_dir_data_t dir_data;
    tmpfs_internal_file_data_t file_data;
  };
} tmpfs_internal_t;

typedef struct {
  const char *component_name;
  struct vnode *vnode;
} tmpfs_dir_contents_entry_t;

static int _tmpfs_setup_mount(struct filesystem *fs, struct mount *mount);

static int _tmpfs_write(struct file *file, const void *buf, size_t len);
static int _tmpfs_read(struct file *file, void *buf, size_t len);
static int _tmpfs_open(struct vnode *file_node, struct file **target);
static int _tmpfs_close(struct file *file);
static long _tmpfs_lseek64(struct file *file, long offset, int whence);

static int _tmpfs_lookup(struct vnode *dir_node, struct vnode **target,
                         const char *component_name);
static int _tmpfs_create(struct vnode *dir_node, struct vnode **target,
                         const char *component_name);
static int _tmpfs_mkdir(struct vnode *dir_node, struct vnode **target,
                        const char *component_name);
static int _tmpfs_mknod(struct vnode *dir_node, struct vnode **target,
                        const char *component_name, struct device *device);

struct filesystem tmpfs = {.name = "tmpfs", .setup_mount = _tmpfs_setup_mount};

static struct file_operations _tmpfs_file_operations = {.write = _tmpfs_write,
                                                        .read = _tmpfs_read,
                                                        .open = _tmpfs_open,
                                                        .close = _tmpfs_close,
                                                        .lseek64 =
                                                            _tmpfs_lseek64};

static struct vnode_operations _tmpfs_vnode_operations = {
    .lookup = _tmpfs_lookup,
    .create = _tmpfs_create,
    .mkdir = _tmpfs_mkdir,
    .mknod = _tmpfs_mknod};

static int
_tmpfs_cmp_dir_contents_entries(const tmpfs_dir_contents_entry_t *const e1,
                                const tmpfs_dir_contents_entry_t *const e2,
                                void *_arg) {
  (void)_arg;

  return strcmp(e1->component_name, e2->component_name);
}

static int _tmpfs_cmp_component_name_and_dir_contents_entry(
    const char *const component_name,
    const tmpfs_dir_contents_entry_t *const entry, void *_arg) {
  (void)_arg;

  return strcmp(component_name, entry->component_name);
}

static struct vnode *_tmpfs_create_file_vnode(struct mount *const mount) {
  struct vnode *const result = malloc(sizeof(struct vnode));
  if (!result)
    return NULL;

  tmpfs_internal_t *const internal = malloc(sizeof(tmpfs_internal_t));
  if (!internal) {
    free(result);
    return NULL;
  }

  const spage_id_t contents_page = alloc_pages(0);
  if (contents_page < 0) {
    free(internal);
    free(result);
    return NULL;
  }
  unsigned char *const contents = pa_to_kernel_va(page_id_to_pa(contents_page));

  *internal = (tmpfs_internal_t){.type = TYPE_FILE,
                                 .file_data = (tmpfs_internal_file_data_t){
                                     .contents = contents, .size = 0}};
  *result = (struct vnode){.mount = mount,
                           .v_ops = &_tmpfs_vnode_operations,
                           .f_ops = &_tmpfs_file_operations,
                           .internal = internal};
  return result;
}

static struct vnode *_tmpfs_create_dir_vnode(struct mount *const mount,
                                             struct vnode *const parent) {
  struct vnode *const result = malloc(sizeof(struct vnode));
  if (!result)
    return NULL;

  tmpfs_internal_t *const internal = malloc(sizeof(tmpfs_internal_t));
  if (!internal) {
    free(result);
    return NULL;
  }

  *internal = (tmpfs_internal_t){.type = TYPE_DIR,
                                 .dir_data = (tmpfs_internal_dir_data_t){
                                     .parent = parent, .contents = NULL}};
  *result = (struct vnode){.mount = mount,
                           .v_ops = &_tmpfs_vnode_operations,
                           .f_ops = &_tmpfs_file_operations,
                           .internal = internal};
  return result;
}

static int _tmpfs_setup_mount(struct filesystem *const fs,
                              struct mount *const mount) {
  struct vnode *const root_vnode = _tmpfs_create_dir_vnode(mount, NULL);
  if (!root_vnode)
    return -ENOMEM;

  *mount = (struct mount){.fs = fs, .root = root_vnode};

  return 0;
}

static int _tmpfs_write(struct file *const file, const void *const buf,
                        const size_t len) {
  tmpfs_internal_t *const internal = (tmpfs_internal_t *)file->vnode->internal;
  if (internal->type != TYPE_FILE)
    return -EISDIR;

  tmpfs_internal_file_data_t *const file_data = &internal->file_data;

  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  if (file->f_pos >= MAX_FILE_SZ) {
    CRITICAL_SECTION_LEAVE(daif_val);
    return len == 0 ? 0 : -EFBIG;
  }

  const size_t remaining_len = MAX_FILE_SZ - file->f_pos,
               cpy_len = len < remaining_len ? len : remaining_len;

  if (file->f_pos > file_data->size) {
    memset(file_data->contents, 0, file->f_pos - file_data->size);
  }

  memcpy(file_data->contents + file->f_pos, buf, cpy_len);
  file->f_pos += cpy_len;
  if (file->f_pos > file_data->size) {
    file_data->size = file->f_pos;
  }

  CRITICAL_SECTION_LEAVE(daif_val);

  return cpy_len;
}

static int _tmpfs_read(struct file *const file, void *const buf,
                       const size_t len) {
  tmpfs_internal_t *const internal = (tmpfs_internal_t *)file->vnode->internal;
  if (internal->type != TYPE_FILE)
    return -EISDIR;

  tmpfs_internal_file_data_t *const file_data = &internal->file_data;

  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  const size_t remaining_len = file->f_pos >= file_data->size
                                   ? 0
                                   : file_data->size - file->f_pos,
               cpy_len = len < remaining_len ? len : remaining_len;

  memcpy(buf, file_data->contents + file->f_pos, cpy_len);
  file->f_pos += cpy_len;

  CRITICAL_SECTION_LEAVE(daif_val);

  return cpy_len;
}

static int _tmpfs_open(struct vnode *const file_node,
                       struct file **const target) {
  tmpfs_internal_t *const internal = (tmpfs_internal_t *)file_node->internal;
  if (internal->type != TYPE_FILE)
    return -EISDIR;

  struct file *const file_handle = malloc(sizeof(struct file));
  if (!file_handle)
    return -ENOMEM;

  *file_handle = (struct file){.vnode = file_node,
                               .f_pos = 0,
                               .f_ops = &_tmpfs_file_operations,
                               .flags = 0};
  *target = file_handle;

  return 0;
}

static int _tmpfs_close(struct file *const file) {
  free(file);
  return 0;
}

static long _tmpfs_lseek64(struct file *const file, const long offset,
                           const int whence) {
  tmpfs_internal_t *const internal = (tmpfs_internal_t *)file->vnode->internal;
  if (internal->type != TYPE_FILE)
    return -EISDIR;

  if (!(whence == SEEK_SET || whence == SEEK_CUR || whence == SEEK_END))
    return -EINVAL;

  const long f_pos_base = whence == SEEK_SET   ? 0
                          : whence == SEEK_CUR ? file->f_pos
                                               : MAX_FILE_SZ,
             new_f_pos = f_pos_base + offset;

  if (new_f_pos < 0) {
    return -EINVAL;
  } else {
    file->f_pos = new_f_pos;
    return 0;
  }
}

static int _tmpfs_lookup(struct vnode *const dir_node,
                         struct vnode **const target,
                         const char *const component_name) {
  tmpfs_internal_t *const internal = (tmpfs_internal_t *)dir_node->internal;
  if (internal->type != TYPE_DIR)
    return -ENOTDIR;

  int result;
  tmpfs_internal_dir_data_t *const dir_data = &internal->dir_data;

  if (strcmp(component_name, ".") == 0) {
    *target = dir_node;
    return 0;
  } else if (strcmp(component_name, "..") == 0) {
    *target = dir_data->parent;
    return 0;
  }

  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  const tmpfs_dir_contents_entry_t *const entry = rb_search(
      dir_data->contents, component_name,
      (int (*)(const void *, const void *,
               void *))_tmpfs_cmp_component_name_and_dir_contents_entry,
      NULL);

  if (entry) {
    *target = entry->vnode;
    result = 0;
  } else {
    result = -ENOENT;
  }

  CRITICAL_SECTION_LEAVE(daif_val);
  return result;
}

static int _tmpfs_create(struct vnode *const dir_node,
                         struct vnode **const target,
                         const char *const component_name) {
  tmpfs_internal_t *const internal = (tmpfs_internal_t *)dir_node->internal;
  if (internal->type != TYPE_DIR)
    return -ENOTDIR;

  int result;
  tmpfs_internal_dir_data_t *const dir_data = &internal->dir_data;

  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  const tmpfs_dir_contents_entry_t *const existing_entry = rb_search(
      dir_data->contents, component_name,
      (int (*)(const void *, const void *,
               void *))_tmpfs_cmp_component_name_and_dir_contents_entry,
      NULL);
  if (existing_entry) {
    result = -EEXIST;
    goto end;
  }

  char *const entry_component_name = strdup(component_name);
  if (!entry_component_name) {
    result = -ENOMEM;
    goto end;
  }

  struct vnode *const vnode = _tmpfs_create_file_vnode(dir_node->mount);
  if (!vnode) {
    free(entry_component_name);
    result = -ENOMEM;
    goto end;
  }

  const tmpfs_dir_contents_entry_t new_entry = {
      .component_name = entry_component_name, .vnode = vnode};
  rb_insert(&dir_data->contents, sizeof(tmpfs_dir_contents_entry_t), &new_entry,
            (int (*)(const void *, const void *,
                     void *))_tmpfs_cmp_dir_contents_entries,
            NULL);

  *target = vnode;
  result = 0;

end:
  CRITICAL_SECTION_LEAVE(daif_val);
  return result;
}

static int _tmpfs_mkdir(struct vnode *const dir_node,
                        struct vnode **const target,
                        const char *const component_name) {
  tmpfs_internal_t *const internal = (tmpfs_internal_t *)dir_node->internal;
  if (internal->type != TYPE_DIR)
    return -ENOTDIR;

  int result;
  tmpfs_internal_dir_data_t *const dir_data = &internal->dir_data;

  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  const tmpfs_dir_contents_entry_t *const existing_entry = rb_search(
      dir_data->contents, component_name,
      (int (*)(const void *, const void *,
               void *))_tmpfs_cmp_component_name_and_dir_contents_entry,
      NULL);
  if (existing_entry) {
    result = -EEXIST;
    goto end;
  }

  char *const entry_component_name = strdup(component_name);
  if (!entry_component_name) {
    result = -ENOMEM;
    goto end;
  }

  struct vnode *const vnode =
      _tmpfs_create_dir_vnode(dir_node->mount, dir_node);
  if (!vnode) {
    free(entry_component_name);
    result = -ENOMEM;
    goto end;
  }

  const tmpfs_dir_contents_entry_t new_entry = {
      .component_name = entry_component_name, .vnode = vnode};
  rb_insert(&dir_data->contents, sizeof(tmpfs_dir_contents_entry_t), &new_entry,
            (int (*)(const void *, const void *,
                     void *))_tmpfs_cmp_dir_contents_entries,
            NULL);

  *target = vnode;
  result = 0;

end:
  CRITICAL_SECTION_LEAVE(daif_val);
  return result;
}

static int _tmpfs_mknod(struct vnode *const dir_node,
                        struct vnode **const target,
                        const char *const component_name,
                        struct device *const device) {
  tmpfs_internal_t *const internal = (tmpfs_internal_t *)dir_node->internal;
  if (internal->type != TYPE_DIR)
    return -ENOTDIR;

  int result;
  tmpfs_internal_dir_data_t *const dir_data = &internal->dir_data;

  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  const tmpfs_dir_contents_entry_t *const existing_entry = rb_search(
      dir_data->contents, component_name,
      (int (*)(const void *, const void *,
               void *))_tmpfs_cmp_component_name_and_dir_contents_entry,
      NULL);
  if (existing_entry) {
    result = -EEXIST;
    goto end;
  }

  char *const entry_component_name = strdup(component_name);
  if (!entry_component_name) {
    result = -ENOMEM;
    goto end;
  }

  struct vnode *const vnode = malloc(sizeof(struct vnode));
  if (!vnode) {
    free(entry_component_name);
    result = -ENOMEM;
    goto end;
  }
  vnode->mount = dir_node->mount;

  if ((result = device->setup_mount(device, vnode)) < 0) {
    free(vnode);
    free(entry_component_name);
    goto end;
  }

  const tmpfs_dir_contents_entry_t new_entry = {
      .component_name = entry_component_name, .vnode = vnode};
  rb_insert(&dir_data->contents, sizeof(tmpfs_dir_contents_entry_t), &new_entry,
            (int (*)(const void *, const void *,
                     void *))_tmpfs_cmp_dir_contents_entries,
            NULL);

  *target = vnode;
  result = 0;

end:
  CRITICAL_SECTION_LEAVE(daif_val);
  return result;
}
