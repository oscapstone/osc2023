#include "oscos/fs/sd-fat32.h"

#include "oscos/console.h"
#include "oscos/drivers/sdhost.h"
#include "oscos/initrd.h"
#include "oscos/libc/ctype.h"
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
  size_t partition_lba, fat_lba_offset, fat_n_sectors, data_region_lba_offset,
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
static int _sd_fat32_ioctl(struct file *file, unsigned long request,
                           void *payload);

static int _sd_fat32_lookup(struct vnode *dir_node, struct vnode **target,
                            const char *component_name);
static int _sd_fat32_create(struct vnode *dir_node, struct vnode **target,
                            const char *component_name);
static int _sd_fat32_mkdir(struct vnode *dir_node, struct vnode **target,
                           const char *component_name);
static int _sd_fat32_mknod(struct vnode *dir_node, struct vnode **target,
                           const char *component_name, struct device *device);
static long _sd_fat32_get_size(struct vnode *vnode);

struct filesystem sd_fat32 = {.name = "fat32",
                              .setup_mount = _sd_fat32_setup_mount};

static struct file_operations _sd_fat32_file_operations = {
    .write = _sd_fat32_write,
    .read = _sd_fat32_read,
    .open = _sd_fat32_open,
    .close = _sd_fat32_close,
    .lseek64 = _sd_fat32_lseek64,
    .ioctl = _sd_fat32_ioctl};

static struct vnode_operations _sd_fat32_vnode_operations = {
    .lookup = _sd_fat32_lookup,
    .create = _sd_fat32_create,
    .mkdir = _sd_fat32_mkdir,
    .mknod = _sd_fat32_mknod,
    .get_size = _sd_fat32_get_size};

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

static uint32_t _sd_fat32_read_fat(const fat32_fsinfo_t *const fsinfo,
                                   const size_t cluster_addr,
                                   unsigned char block_buf[static 512]) {
  readblock(_sd_fat32_cluster_addr_to_fat_lba(fsinfo, cluster_addr), block_buf);
  const uint32_t *const fat_entries = (const uint32_t *)block_buf;
  return fat_entries[cluster_addr & ((1 << 7) - 1)] & 0x0fffffff;
}

static void _sd_fat32_write_fat(const fat32_fsinfo_t *const fsinfo,
                                const size_t cluster_addr, const uint32_t value,
                                unsigned char block_buf[static 512]) {
  readblock(_sd_fat32_cluster_addr_to_fat_lba(fsinfo, cluster_addr), block_buf);
  uint32_t *const fat_entries = (uint32_t *)block_buf;
  fat_entries[cluster_addr & ((1 << 7) - 1)] = value;
  writeblock(_sd_fat32_cluster_addr_to_fat_lba(fsinfo, cluster_addr),
             block_buf);
}

static bool _sd_fat32_is_component_name_char_valid_lax(const char c) {
  const unsigned char u = c;
  return isalnum(c) || strchr(" !#$%&'()-@^_`{}~", c) || u >= 128;
}

static char _sd_fat32_map_component_name_char(const char c) {
  if (!_sd_fat32_is_component_name_char_valid_lax(c))
    __builtin_unreachable();

  return islower(c) ? toupper(c) : (unsigned char)c == 0xe5 ? 0x05 : c;
}

static bool _sd_fat32_check_and_map_filename(const char *const filename,
                                             char target[static 11]) {
  const char *const last_dot = strrchr(filename, '.');
  const size_t noext_len =
      last_dot ? (size_t)(last_dot - filename) : strlen(filename);
  if (noext_len > 8)
    return false;
  const size_t ext_len = last_dot ? strlen(filename) - noext_len - 1 : 0;
  if (ext_len > 3)
    return false;

  for (const char *c = filename; *c; c++) {
    if (c != last_dot && !_sd_fat32_is_component_name_char_valid_lax(*c))
      return false;
  }

  for (size_t i = 0; i < 8; i++) {
    target[i] =
        i < noext_len ? _sd_fat32_map_component_name_char(filename[i]) : ' ';
  }
  for (size_t i = 0; i < 3; i++) {
    target[i + 8] =
        i < ext_len ? _sd_fat32_map_component_name_char(last_dot[1 + i]) : ' ';
  }

  return true;
}

static size_t _held_cluster_addr = -1;

static size_t _sd_fat32_find_free_cluster(const fat32_fsinfo_t *const fsinfo,
                                          unsigned char block_buf[static 512]) {
  const size_t max_n_clusters = fsinfo->fat_n_sectors << 7;
  for (size_t cluster_addr = 0; cluster_addr < max_n_clusters; cluster_addr++) {
    if (cluster_addr != _held_cluster_addr &&
        _sd_fat32_read_fat(fsinfo, cluster_addr, block_buf) == 0) {
      return cluster_addr;
    }
  }
  return -1;
}

static size_t
_sd_fat32_find_free_cluster_and_hold(const fat32_fsinfo_t *const fsinfo,
                                     unsigned char block_buf[static 512]) {
  return _held_cluster_addr = _sd_fat32_find_free_cluster(fsinfo, block_buf);
}

static size_t _sd_fat32_alloc_free_cluster_and_extend_chain(
    const fat32_fsinfo_t *const fsinfo, const size_t chain_end_cluster_addr,
    unsigned char block_buf[static 512]) {
  const size_t new_cluster_addr =
      _sd_fat32_find_free_cluster(fsinfo, block_buf);
  if (new_cluster_addr == (size_t)-1)
    return -1;

  _sd_fat32_write_fat(fsinfo, new_cluster_addr, 0x0fffffff, block_buf);
  if (chain_end_cluster_addr != (size_t)-1) {
    _sd_fat32_write_fat(fsinfo, chain_end_cluster_addr, new_cluster_addr,
                        block_buf);
  }

  return new_cluster_addr;
}

static void _sd_fat32_alloc_held_cluster(const fat32_fsinfo_t *const fsinfo,
                                         unsigned char block_buf[static 512]) {
  _sd_fat32_write_fat(fsinfo, _held_cluster_addr, 0x0fffffff, block_buf);
}

static void _sd_fat32_unhold_cluster(void) { _held_cluster_addr = -1; }

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
                             .fat_n_sectors = bpb->spf32,
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
  sd_fat32_internal_t *const internal =
      (sd_fat32_internal_t *)file->vnode->internal;
  if (internal->type != TYPE_FILE)
    return -EISDIR;

  sd_fat32_internal_file_data_t *const file_data = &internal->file_data;
  const fat32_fsinfo_t *const fsinfo =
      ((sd_fat32_internal_t *)(file->vnode->mount->root->internal))->fsinfo;

  unsigned char *const block_buf = malloc(512);
  if (!block_buf)
    return -ENOMEM;

  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  const size_t write_end_offset = file->f_pos + len;

  size_t curr_cluster_addr = file_data->cluster_addr, file_offset = 0,
         n_chars_written = 0;
  for (;;) {
    for (size_t sector_of_cluster = 0;
         sector_of_cluster < fsinfo->cluster_n_sectors; sector_of_cluster++) {
      const size_t end_offset = file_offset + 512;
      const size_t max_start =
                       file_offset > file->f_pos ? file_offset : file->f_pos,
                   min_end = end_offset < write_end_offset ? end_offset
                                                           : write_end_offset;
      if (max_start < min_end) {
        readblock(
            _sd_fat32_cluster_addr_to_data_lba(fsinfo, curr_cluster_addr) +
                sector_of_cluster,
            block_buf);
        memcpy(block_buf + (max_start & 511), (char *)buf + n_chars_written,
               min_end - max_start);
        writeblock(
            _sd_fat32_cluster_addr_to_data_lba(fsinfo, curr_cluster_addr) +
                sector_of_cluster,
            block_buf);
        n_chars_written += min_end - max_start;
      }

      file_offset += 512;
      if (file_offset >= write_end_offset) // Done writing.
        break;
    }

    if (file_offset >= write_end_offset) // Done writing.
      break;

    const uint32_t fat_entry =
        _sd_fat32_read_fat(fsinfo, curr_cluster_addr, block_buf);
    if (!(0x2 <= fat_entry && fat_entry <= 0x0ffffff7)) { // No more chains.
      curr_cluster_addr = _sd_fat32_alloc_free_cluster_and_extend_chain(
          fsinfo, curr_cluster_addr, block_buf);
      if (curr_cluster_addr == (size_t)-1) { // Out of space.
        free(block_buf);
        CRITICAL_SECTION_LEAVE(daif_val);
        return -ENOSPC;
      }

      for (size_t sector_of_cluster = 0;
           sector_of_cluster < fsinfo->cluster_n_sectors; sector_of_cluster++) {
        memset(block_buf, 0, 512);
        writeblock(
            _sd_fat32_cluster_addr_to_data_lba(fsinfo, curr_cluster_addr) +
                sector_of_cluster,
            block_buf);
      }
    } else {
      curr_cluster_addr = fat_entry;
    }
  }

  file->f_pos += n_chars_written;
  if (file_data->size > file->f_pos) {
    file_data->size = file->f_pos;
  }

  free(block_buf);
  CRITICAL_SECTION_LEAVE(daif_val);
  return n_chars_written;
}

static int _sd_fat32_read(struct file *const file, void *const buf,
                          const size_t len) {
  sd_fat32_internal_t *const internal =
      (sd_fat32_internal_t *)file->vnode->internal;
  if (internal->type != TYPE_FILE)
    return -EISDIR;

  sd_fat32_internal_file_data_t *const file_data = &internal->file_data;
  const fat32_fsinfo_t *const fsinfo =
      ((sd_fat32_internal_t *)(file->vnode->mount->root->internal))->fsinfo;

  unsigned char *const block_buf = malloc(512);
  if (!block_buf)
    return -ENOMEM;

  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  const size_t read_end_offset =
      file->f_pos + len < file_data->size ? file->f_pos + len : file_data->size;

  size_t curr_cluster_addr = file_data->cluster_addr, file_offset = 0,
         n_chars_read = 0;
  for (;;) {
    for (size_t sector_of_cluster = 0;
         sector_of_cluster < fsinfo->cluster_n_sectors; sector_of_cluster++) {
      const size_t end_offset = file_offset + 512;
      const size_t max_start =
                       file_offset > file->f_pos ? file_offset : file->f_pos,
                   min_end = end_offset < read_end_offset ? end_offset
                                                          : read_end_offset;
      if (max_start < min_end) {
        readblock(
            _sd_fat32_cluster_addr_to_data_lba(fsinfo, curr_cluster_addr) +
                sector_of_cluster,
            block_buf);
        memcpy((char *)buf + n_chars_read, block_buf + (max_start & 511),
               min_end - max_start);
        n_chars_read += min_end - max_start;
      }

      file_offset += 512;
      if (file_offset >= read_end_offset) // Done reading.
        break;
    }

    if (file_offset >= read_end_offset) // Done reading.
      break;

    const uint32_t fat_entry =
        _sd_fat32_read_fat(fsinfo, curr_cluster_addr, block_buf);
    if (!(0x2 <= fat_entry && fat_entry <= 0x0ffffff7)) { // No more chains.
      // The file has a hole, which should read zero.
      memset(block_buf + n_chars_read, 0, read_end_offset - file_offset);
      n_chars_read += read_end_offset - file_offset;
      break;
    }

    curr_cluster_addr = fat_entry;
  }

  file->f_pos += n_chars_read;

  free(block_buf);
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
}

static int _sd_fat32_ioctl(struct file *const file, const unsigned long request,
                           void *const payload) {
  (void)file;
  (void)request;
  (void)payload;

  return -ENOTTY;
}

static int _sd_fat32_lookup(struct vnode *const dir_node,
                            struct vnode **const target,
                            const char *const component_name) {
  sd_fat32_internal_t *const internal =
      (sd_fat32_internal_t *)dir_node->internal;
  if (internal->type != TYPE_DIR)
    return -ENOTDIR;

  sd_fat32_internal_dir_data_t *const dir_data = &internal->dir_data;
  const fat32_fsinfo_t *const fsinfo =
      ((sd_fat32_internal_t *)(dir_node->mount->root->internal))->fsinfo;

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

  // A vnode has not been created before.
  // Check the filename.

  char filename_buf[11];
  if (!_sd_fat32_check_and_map_filename(
          component_name, filename_buf)) // Invalid component name.
    return -ENOENT;

  unsigned char *const block_buf = malloc(512);
  if (!block_buf)
    return -ENOMEM;

  size_t curr_cluster_addr = dir_data->cluster_addr;
  const fatdir_t *dir_entry = NULL;
  bool done = false;
  while (!done) {
    for (size_t sector_of_cluster = 0;
         sector_of_cluster < fsinfo->cluster_n_sectors; sector_of_cluster++) {
      readblock(_sd_fat32_cluster_addr_to_data_lba(fsinfo, curr_cluster_addr) +
                    sector_of_cluster,
                block_buf);

      for (size_t offset = 0; offset < 512; offset += sizeof(fatdir_t)) {
        const fatdir_t *const curr_dir_entry =
            (const fatdir_t *)(block_buf + offset);
        if (curr_dir_entry->name[0] == 0) {
          done = true;
          break;
        }

        if (curr_dir_entry->name[0] == 0xe5 ||
            curr_dir_entry->attr[0] == 0xf) // Invalid entry.
          continue;

        if (memcmp(curr_dir_entry->name, filename_buf, 11) == 0) {
          dir_entry = curr_dir_entry;
          done = true;
          break;
        }
      }
    }

    if (done)
      break;

    const uint32_t fat_entry =
        _sd_fat32_read_fat(fsinfo, curr_cluster_addr, block_buf);
    if (!(0x2 <= fat_entry && fat_entry <= 0x0ffffff7)) // Nothing more to read.
      break;

    curr_cluster_addr = fat_entry;
  }

  if (!dir_entry) {
    free(block_buf);
    return -ENOENT;
  }

  // Create a new vnode.

  const bool is_dir = dir_entry->attr[0] & 0x10;
  const size_t cluster_addr = ((size_t)dir_entry->ch << 16) + dir_entry->cl;
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

static int _sd_fat32_create_impl(struct vnode *const dir_node,
                                 struct vnode **const target,
                                 const char *const component_name,
                                 const bool is_creating_dir) {
  sd_fat32_internal_t *const internal =
      (sd_fat32_internal_t *)dir_node->internal;
  if (internal->type != TYPE_DIR)
    return -ENOTDIR;

  sd_fat32_internal_dir_data_t *const dir_data = &internal->dir_data;
  const fat32_fsinfo_t *const fsinfo =
      ((sd_fat32_internal_t *)(dir_node->mount->root->internal))->fsinfo;

  // Check the filename.

  if (strcmp(component_name, ".") == 0) {
    return -EINVAL;
  } else if (strcmp(component_name, "..") == 0) {
    return -EINVAL;
  }

  char filename_buf[11];
  if (!_sd_fat32_check_and_map_filename(component_name, filename_buf))
    return -EINVAL;

  // Check if the file already exists.

  if (_sd_fat32_lookup(dir_node, target, component_name) == 0)
    return -EEXIST;

  unsigned char *const block_buf = malloc(512);
  if (!block_buf)
    return -ENOMEM;

  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  // Find an empty entry in the FAT.

  const size_t new_file_cluster_addr =
      _sd_fat32_find_free_cluster_and_hold(fsinfo, block_buf);
  if (new_file_cluster_addr == (size_t)-1) {
    free(block_buf);
    CRITICAL_SECTION_LEAVE(daif_val);
    return -ENOSPC;
  }

  // Allocate things early,
  // so that we won't have to unroll a lot of FS state later.

  struct vnode *const vnode =
      is_creating_dir ? _sd_fat32_create_dir_vnode(dir_node->mount, dir_node,
                                                   new_file_cluster_addr)
                      : _sd_fat32_create_file_vnode(dir_node->mount,
                                                    new_file_cluster_addr, 0);
  if (!vnode) {
    _sd_fat32_unhold_cluster();
    free(block_buf);
    CRITICAL_SECTION_LEAVE(daif_val);
    return -ENOMEM;
  }

  char *const entry_component_name = strdup(component_name);
  if (!entry_component_name) {
    free(vnode);
    _sd_fat32_unhold_cluster();
    free(block_buf);
    CRITICAL_SECTION_LEAVE(daif_val);
    return -ENOMEM;
  }

  // Find an empty directory entry in the target directory.

  size_t dir_entry_cluster_addr = dir_data->cluster_addr;
  fatdir_t *dir_entry = NULL, *dir_entry_to_zero = NULL;
  bool done = false;
  while (!done) {
    for (size_t sector_of_cluster = 0;
         sector_of_cluster < fsinfo->cluster_n_sectors; sector_of_cluster++) {
      readblock(
          _sd_fat32_cluster_addr_to_data_lba(fsinfo, dir_entry_cluster_addr) +
              sector_of_cluster,
          block_buf);

      for (size_t offset = 0; offset < 512; offset += sizeof(fatdir_t)) {
        fatdir_t *const curr_dir_entry = (fatdir_t *)(block_buf + offset);
        if (curr_dir_entry->name[0] == 0) {
          dir_entry = curr_dir_entry;
          done = true;
          dir_entry_to_zero = curr_dir_entry + 1;
          if ((unsigned char *)dir_entry_to_zero - block_buf >= 512)
            dir_entry_to_zero = NULL;
          break;
        }

        if (curr_dir_entry->name[0] == 0xe5 ||
            curr_dir_entry->attr[0] == 0xf) { // Invalid entry.
          dir_entry = curr_dir_entry;
          done = true;
          break;
        }
      }
    }

    if (done)
      break;

    const uint32_t fat_entry =
        _sd_fat32_read_fat(fsinfo, dir_entry_cluster_addr, block_buf);
    if (!(0x2 <= fat_entry && fat_entry <= 0x0ffffff7)) // Nothing more to read.
      break;

    dir_entry_cluster_addr = fat_entry;
  }

  if (!dir_entry) {
    dir_entry_cluster_addr = _sd_fat32_alloc_free_cluster_and_extend_chain(
        fsinfo, dir_entry_cluster_addr, block_buf);
    if (dir_entry_cluster_addr == (size_t)-1) {
      free(entry_component_name);
      free(vnode);
      _sd_fat32_unhold_cluster();
      free(block_buf);
      CRITICAL_SECTION_LEAVE(daif_val);
      return -ENOSPC;
    }

    memset(block_buf, 0, 512);
    dir_entry = (fatdir_t *)block_buf;
    done = true;
  }

  // Set the data in the new directory entry.

  memcpy(dir_entry->name, filename_buf, 11);
  // Can't set the timestamps properly, since this kernel doesn't keep time.
  memset(dir_entry->attr, 0, 9);
  dir_entry->ch = new_file_cluster_addr >> 16;
  dir_entry->attr2 = 0;
  dir_entry->cl = new_file_cluster_addr & ((1 << 16) - 1);
  dir_entry->size = 0;

  if (dir_entry_to_zero) {
    dir_entry_to_zero->name[0] = 0;
  }

  writeblock(dir_entry_cluster_addr, block_buf);

  _sd_fat32_alloc_held_cluster(fsinfo, block_buf);

  free(block_buf);

  // Insert the vnode.

  const sd_fat32_child_vnode_entry_t new_child_vnode_entry = {
      .component_name = entry_component_name, .vnode = vnode};

  rb_insert(&dir_data->child_vnodes, sizeof(sd_fat32_child_vnode_entry_t),
            &new_child_vnode_entry,
            (int (*)(const void *, const void *, void *))
                _sd_fat32_cmp_child_vnode_entries_by_component_name,
            NULL);

  *target = vnode;

  CRITICAL_SECTION_LEAVE(daif_val);
  return 0;
}

static int _sd_fat32_create(struct vnode *const dir_node,
                            struct vnode **const target,
                            const char *const component_name) {
  return _sd_fat32_create_impl(dir_node, target, component_name, false);
}

static int _sd_fat32_mkdir(struct vnode *const dir_node,
                           struct vnode **const target,
                           const char *const component_name) {
  return _sd_fat32_create_impl(dir_node, target, component_name, true);
}

static int _sd_fat32_mknod(struct vnode *const dir_node,
                           struct vnode **const target,
                           const char *const component_name,
                           struct device *const device) {
  (void)dir_node;
  (void)target;
  (void)component_name;
  (void)device;

  return -EPERM;
}

static long _sd_fat32_get_size(struct vnode *const vnode) {
  sd_fat32_internal_t *const internal = (sd_fat32_internal_t *)vnode->internal;
  if (internal->type != TYPE_FILE)
    return -EISDIR;

  sd_fat32_internal_file_data_t *const file_data = &internal->file_data;

  return file_data->size;
}
