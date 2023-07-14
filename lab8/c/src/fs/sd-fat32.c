#include "oscos/fs/initramfs.h"

#include "oscos/console.h"
#include "oscos/drivers/sdhost.h"
#include "oscos/initrd.h"
#include "oscos/libc/string.h"
#include "oscos/mem/malloc.h"
#include "oscos/uapi/errno.h"
#include "oscos/uapi/unistd.h"
#include "oscos/utils/critical-section.h"
#include "oscos/utils/rb.h"

typedef struct {
  char jmp[3];
  char oem[8];
  unsigned char bps0;
  unsigned char bps1;
  unsigned char spc;
  unsigned short rsc;
  unsigned char nf;
  unsigned char nr0;
  unsigned char nr1;
  unsigned short ts16;
  unsigned char media;
  unsigned short spf16;
  unsigned short spt;
  unsigned short nh;
  unsigned int hs;
  unsigned int ts32;
  unsigned int spf32;
  unsigned int flg;
  unsigned int rc;
  char vol[6];
  char fst[8];
  char dmy[20];
  char fst2[8];
} __attribute__((packed)) bpb_t;

typedef struct {
  char name[8];
  char ext[3];
  char attr[9];
  unsigned short ch;
  unsigned int attr2;
  unsigned short cl;
  unsigned int size;
} __attribute__((packed)) fatdir_t;

typedef struct {
  size_t partition_lba, fat_lba_offset, data_region_lba_offset,
      cluster_n_sectors;
} fat32_fsinfo_t;

typedef struct {
  const char *component_name;
  struct vnode *vnode;
} sd_fat32_child_vnode_entry_t;

typedef enum { TYPE_DIR, TYPE_FILE } sd_fat32_internal_type_t;

typedef struct {
  struct vnode *parent;
  size_t cluster_addr;
  rb_node_t *child_vnodes; // Created lazily.
} sd_fat32_internal_dir_data_t;

typedef struct {
  size_t cluster_addr;
  size_t size;
} sd_fat32_internal_file_data_t;

typedef struct {
  sd_fat32_internal_type_t type;
  const fat32_fsinfo_t *fsinfo;
  union {
    sd_fat32_internal_file_data_t file_data;
    sd_fat32_internal_dir_data_t dir_data;
  };
} sd_fat32_internal_t;

static int _sd_fat32_setup_mount(struct filesystem *fs, struct mount *mount);

static int _sd_fat32_write(struct file *file, const void *buf, size_t len);
static int _sd_fat32_read(struct file *file, void *buf, size_t len);
static int _sd_fat32_open(struct vnode *file_node, struct file **target);
static int _sd_fat32_close(struct file *file);
static long _sd_fat32_lseek64(struct file *file, long offset, int whence);

static int _sd_fat32_lookup(struct vnode *dir_node, struct vnode **target,
                            const char *component_name);
static int _sd_fat32_create(struct vnode *dir_node, struct vnode **target,
                            const char *component_name);
static int _sd_fat32_mkdir(struct vnode *dir_node, struct vnode **target,
                           const char *component_name);

struct filesystem sd_fat32 = {.name = "fat32",
                              .setup_mount = _sd_fat32_setup_mount};

static struct file_operations _sd_fat32_file_operations = {
    .write = _sd_fat32_write,
    .read = _sd_fat32_read,
    .open = _sd_fat32_open,
    .close = _sd_fat32_close,
    .lseek64 = _sd_fat32_lseek64};

static struct vnode_operations _sd_fat32_vnode_operations = {
    .lookup = _sd_fat32_lookup,
    .create = _sd_fat32_create,
    .mkdir = _sd_fat32_mkdir};

static size_t
_sd_fat32_cluster_addr_to_data_lba(const fat32_fsinfo_t *const fsinfo,
                                   const size_t cluster_addr) {
  return fsinfo->partition_lba + fsinfo->data_region_lba_offset +
         cluster_addr * fsinfo->cluster_n_sectors;
}

static size_t
_sd_fat32_cluster_addr_to_fat_lba(const fat32_fsinfo_t *const fsinfo,
                                  const size_t cluster_addr) {
  // Each block has 2⁹⁻² FAT entries.
  return fsinfo->partition_lba + fsinfo->fat_lba_offset + (cluster_addr >> 7);
}

static int _sd_fat32_cmp_child_vnode_entries_by_component_name(
    const sd_fat32_child_vnode_entry_t *const e1,
    const sd_fat32_child_vnode_entry_t *const e2, void *const _arg) {
  (void)_arg;

  return strcmp(e1->component_name, e2->component_name);
}

static int _sd_fat32_cmp_component_name_and_child_vnode_entry(
    const char *component_name, const sd_fat32_child_vnode_entry_t *const entry,
    void *const _arg) {
  (void)_arg;

  return strcmp(component_name, entry->component_name);
}

static struct vnode *_sd_fat32_create_dir_vnode(struct mount *const mount,
                                                struct vnode *const parent,
                                                const size_t cluster_addr) {
  struct vnode *const result = malloc(sizeof(struct vnode));
  if (!result)
    return NULL;

  sd_fat32_internal_t *const internal = malloc(sizeof(sd_fat32_internal_t));
  if (!internal) {
    free(result);
    return NULL;
  }

  *internal = (sd_fat32_internal_t){
      .type = TYPE_DIR,
      .dir_data = (sd_fat32_internal_dir_data_t){.parent = parent,
                                                 .cluster_addr = cluster_addr,
                                                 .child_vnodes = NULL}};
  *result = (struct vnode){.mount = mount,
                           .v_ops = &_sd_fat32_vnode_operations,
                           .f_ops = &_sd_fat32_file_operations,
                           .internal = internal};
  return result;
}

static struct vnode *_sd_fat32_create_file_vnode(struct mount *const mount,
                                                 const size_t cluster_addr,
                                                 const size_t size) {
  struct vnode *const result = malloc(sizeof(struct vnode));
  if (!result)
    return NULL;

  sd_fat32_internal_t *const internal = malloc(sizeof(sd_fat32_internal_t));
  if (!internal) {
    free(result);
    return NULL;
  }

  *internal =
      (sd_fat32_internal_t){.type = TYPE_FILE,
                            .file_data = (sd_fat32_internal_file_data_t){
                                .cluster_addr = cluster_addr, .size = size}};
  *result = (struct vnode){.mount = mount,
                           .v_ops = &_sd_fat32_vnode_operations,
                           .f_ops = &_sd_fat32_file_operations,
                           .internal = internal};
  return result;
}

static int _sd_fat32_setup_mount(struct filesystem *const fs,
                                 struct mount *const mount) {
  unsigned char *const block_buf = malloc(512);
  if (!block_buf)
    return -ENOMEM;

  fat32_fsinfo_t *const fsinfo = malloc(sizeof(fat32_fsinfo_t));
  if (!fsinfo) {
    free(block_buf);
    return -ENOMEM;
  }

  // Read MBR.

  readblock(0, block_buf);

  const size_t partition_lba = block_buf[0x1c6] + (block_buf[0x1c7] << 8) +
                               (block_buf[0x1c8] << 16) +
                               (block_buf[0x1c9] << 24);
  console_printf("DEBUG: sd-fat32: Partition LBA = 0x%zx\n", partition_lba);

  // Read VBR.

  readblock(partition_lba, block_buf);

  bpb_t *const bpb = (bpb_t *)block_buf;
  *fsinfo = (fat32_fsinfo_t){.partition_lba = partition_lba,
                             .fat_lba_offset = bpb->rsc,
                             .data_region_lba_offset =
                                 bpb->rsc + bpb->nf * bpb->spf32 - 2 * bpb->spc,
                             .cluster_n_sectors = bpb->spc};

  const size_t root_cluster_addr = bpb->rc;
  console_printf("DEBUG: sd-fat32: Root dir cluster addr = 0x%zx\n",
                 root_cluster_addr);

  // Create vnode.

  struct vnode *const root_vnode =
      _sd_fat32_create_dir_vnode(mount, NULL, root_cluster_addr);
  if (!root_vnode) {
    free(fsinfo);
    free(block_buf);
    return -ENOMEM;
  }

  ((sd_fat32_internal_t *)root_vnode->internal)->fsinfo = fsinfo;
  *mount = (struct mount){.fs = fs, .root = root_vnode};

  free(block_buf);
  return 0;
}

static int _sd_fat32_write(struct file *const file, const void *const buf,
                           const size_t len) {
  (void)file;
  (void)buf;
  (void)len;

  return -EROFS;
}

static int _sd_fat32_read(struct file *const file, void *const buf,
                          const size_t len) {
  sd_fat32_internal_t *const internal =
      (sd_fat32_internal_t *)file->vnode->internal;
  if (internal->type != TYPE_FILE)
    return -EISDIR;

  sd_fat32_internal_file_data_t *const file_data = &internal->file_data;

  unsigned char *const block_buf = malloc(512);
  if (!block_buf)
    return -ENOMEM;

  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  size_t curr_cluster_addr = file_data->cluster_addr, file_offset = 0,
         n_chars_read = 0;
  for (;;) {
    // Read the next cluster address.

    readblock(_sd_fat32_cluster_addr_to_fat_lba(
                  ((sd_fat32_internal_t *)(file->vnode->mount->root->internal))
                      ->fsinfo,
                  curr_cluster_addr),
              block_buf);
    const uint32_t *const fat_entries = (const uint32_t *)block_buf;
    const size_t next_cluster_addr =
        fat_entries[curr_cluster_addr >> 7] & 0x0fffffff;

    const size_t end_offset = file_offset + 512 > file_data->size
                                  ? file_data->size
                                  : file_offset + 512;
    const size_t max_start =
                     file_offset > file->f_pos ? file_offset : file->f_pos,
                 min_end = end_offset < file->f_pos + len ? end_offset
                                                          : file->f_pos + len;
    if (max_start < min_end) {
      readblock(
          _sd_fat32_cluster_addr_to_data_lba(
              ((sd_fat32_internal_t *)(file->vnode->mount->root->internal))
                  ->fsinfo,
              curr_cluster_addr),
          block_buf);
      memcpy((char *)buf + n_chars_read, block_buf + (max_start & 511),
             min_end - max_start);
      n_chars_read += min_end - max_start;
    }

    if (file_offset + 512 >= file->f_pos + len) // Done reading.
      break;
    if (!(1 < next_cluster_addr && next_cluster_addr &&
          0xfffffff8)) // Nothing more to read.
      break;

    curr_cluster_addr = next_cluster_addr;
    file_offset += 512;
  }

  file->f_pos += n_chars_read;

  CRITICAL_SECTION_LEAVE(daif_val);
  return n_chars_read;
}

static int _sd_fat32_open(struct vnode *const file_node,
                          struct file **const target) {
  sd_fat32_internal_t *const internal =
      (sd_fat32_internal_t *)file_node->internal;
  if (internal->type != TYPE_FILE)
    return -EISDIR;

  struct file *const file_handle = malloc(sizeof(struct file));
  if (!file_handle)
    return -ENOMEM;

  *file_handle = (struct file){.vnode = file_node,
                               .f_pos = 0,
                               .f_ops = &_sd_fat32_file_operations,
                               .flags = 0};
  *target = file_handle;

  return 0;
}

static int _sd_fat32_close(struct file *const file) {
  free(file);
  return 0;
}

static long _sd_fat32_lseek64(struct file *const file, const long offset,
                              const int whence) {
  sd_fat32_internal_t *const internal =
      (sd_fat32_internal_t *)file->vnode->internal;
  if (internal->type != TYPE_FILE)
    return -EISDIR;

  sd_fat32_internal_file_data_t *const file_data = &internal->file_data;

  if (!(whence == SEEK_SET || whence == SEEK_CUR || whence == SEEK_END))
    return -EINVAL;

  const long f_pos_base = whence == SEEK_SET   ? 0
                          : whence == SEEK_CUR ? file->f_pos
                                               : file_data->size,
             new_f_pos = f_pos_base + offset;

  if (new_f_pos < 0) {
    return -EINVAL;
  } else {
    file->f_pos = new_f_pos;
    return 0;
  }

  return -ENOSYS;
}

static int _sd_fat32_lookup(struct vnode *const dir_node,
                            struct vnode **const target,
                            const char *const component_name) {
  sd_fat32_internal_t *const internal =
      (sd_fat32_internal_t *)dir_node->internal;
  if (internal->type != TYPE_DIR)
    return -ENOTDIR;

  sd_fat32_internal_dir_data_t *const dir_data = &internal->dir_data;

  if (strcmp(component_name, ".") == 0) {
    *target = dir_node;
    return 0;
  } else if (strcmp(component_name, "..") == 0) {
    *target = dir_data->parent;
    return 0;
  }

  // Check if the vnode has been created before.

  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  const sd_fat32_child_vnode_entry_t *const child_vnode_entry = rb_search(
      dir_data->child_vnodes, component_name,
      (int (*)(const void *, const void *,
               void *))_sd_fat32_cmp_component_name_and_child_vnode_entry,
      NULL);

  CRITICAL_SECTION_LEAVE(daif_val);

  if (child_vnode_entry) {
    *target = child_vnode_entry->vnode;
    return 0;
  }

  // A vnode has not been created before. Check if the file/directory exists.

  char filename_buf[11] = "           ";

  const char *const component_name_last_dot_ptr = strrchr(component_name, '.');
  const size_t component_name_noext_len =
                   component_name_last_dot_ptr
                       ? (size_t)(component_name_last_dot_ptr - component_name)
                       : strlen(component_name),
               component_name_noext_cpy_len =
                   component_name_noext_len > 8 ? 8 : component_name_noext_len;
  memcpy(filename_buf, component_name, component_name_noext_cpy_len);

  const size_t component_name_ext_len =
                   component_name_last_dot_ptr
                       ? strlen(component_name_last_dot_ptr + 1)
                       : 0,
               component_name_ext_cpy_len =
                   component_name_ext_len > 3 ? 3 : component_name_ext_len;
  memcpy(filename_buf + 8, component_name_last_dot_ptr + 1,
         component_name_ext_cpy_len);

  unsigned char *const block_buf = malloc(512);
  if (!block_buf)
    return -ENOMEM;

  readblock(
      _sd_fat32_cluster_addr_to_data_lba(
          ((sd_fat32_internal_t *)(dir_node->mount->root->internal))->fsinfo,
          dir_data->cluster_addr),
      block_buf);

  const fatdir_t *dir_entry = NULL;
  for (size_t offset = 0; offset < 512; offset += sizeof(fatdir_t)) {
    const fatdir_t *const curr_dir_entry =
        (const fatdir_t *)(block_buf + offset);
    if (curr_dir_entry->name[0] == 0)
      break;

    if (curr_dir_entry->name[0] == 0xe5 ||
        curr_dir_entry->attr[0] == 0xf) // Invalid entry.
      continue;

    if (memcmp(curr_dir_entry->name, filename_buf, 11) == 0) {
      dir_entry = curr_dir_entry;
      break;
    }
  }

  if (!dir_entry) {
    free(block_buf);
    return -ENOENT;
  }

  // Create a new vnode.

  const bool is_dir = dir_entry->attr[0] & 0x10;
  const size_t cluster_addr = (dir_entry->ch << 16) + dir_entry->cl;
  const size_t file_size = dir_entry->size;
  free(block_buf);
  struct vnode *const vnode =
      is_dir
          ? _sd_fat32_create_dir_vnode(dir_node->mount, dir_node, cluster_addr)
          : _sd_fat32_create_file_vnode(dir_node->mount, cluster_addr,
                                        file_size);
  if (!vnode)
    return -ENOMEM;

  const char *const entry_component_name = strdup(component_name);
  if (!entry_component_name) {
    free(vnode);
    return -ENOMEM;
  }

  const sd_fat32_child_vnode_entry_t new_child_vnode_entry = {
      .component_name = entry_component_name, .vnode = vnode};

  CRITICAL_SECTION_ENTER(daif_val);

  rb_insert(&dir_data->child_vnodes, sizeof(sd_fat32_child_vnode_entry_t),
            &new_child_vnode_entry,
            (int (*)(const void *, const void *, void *))
                _sd_fat32_cmp_child_vnode_entries_by_component_name,
            NULL);

  CRITICAL_SECTION_LEAVE(daif_val);

  *target = vnode;

  return 0;
}

static int _sd_fat32_create(struct vnode *const dir_node,
                            struct vnode **const target,
                            const char *const component_name) {
  (void)dir_node;
  (void)target;
  (void)component_name;

  return -EROFS;
}

static int _sd_fat32_mkdir(struct vnode *const dir_node,
                           struct vnode **const target,
                           const char *const component_name) {
  (void)dir_node;
  (void)target;
  (void)component_name;

  return -EROFS;
}
