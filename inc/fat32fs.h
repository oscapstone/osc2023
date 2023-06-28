#ifndef _FAT32FS_H
#define _FAT32FS_H

#include <vfs.h>
#include <type.h>

#define BLOCK_SIZE 512
#define CLUSTER_ENTRY_PER_BLOCK (BLOCK_SIZE / sizeof(struct cluster_entry_t))
#define DIR_PER_BLOCK (BLOCK_SIZE / sizeof(struct dir_t))
#define FAT_DIR     1
#define FAT_FILE    2
#define INVALID_CID 0x0ffffff8

#define ATTR_READ_ONLY  0x01
#define ATTR_HIDDEN     0x02
#define ATTR_SYSTEM     0x04
#define ATTR_VOLUME_ID  0x08
#define ATTR_LFN        0x0f
#define ATTR_DIRECTORY  0x10
#define ATTR_ARCHIVE    0x20
#define ATTR_FILE_DIR_MASK (ATTR_DIRECTORY | ATTR_ARCHIVE)

struct partition_t {
    uint8 status;
    uint8 chss_head;
    uint8 chss_sector;
    uint8 chss_cylinder;
    uint8 type;
    uint8 chse_head;
    uint8 chse_sector;
    uint8 chse_cylinder;
    uint32 lba;
    uint32 sectors;
} __attribute__((packed));

struct boot_sector_t {
    uint8 jmpboot[3];
    uint8 oemname[8];
    uint16 bytes_per_sector;
    uint8 sector_per_cluster;
    uint16 reserved_sector_cnt;
    uint8 fat_cnt;
    uint16 root_entry_cnt;
    uint16 old_sector_cnt;
    uint8 media;
    uint16 sector_per_fat16;
    uint16 sector_per_track;
    uint16 head_cnt;
    uint32 hidden_sector_cnt;
    uint32 sector_cnt;
    uint32 sector_per_fat32;
    uint16 extflags;
    uint16 ver;
    uint32 root_cluster;
    uint16 info;
    uint16 bkbooksec;
    uint8 reserved[12];
    uint8 drvnum;
    uint8 reserved1;
    uint8 bootsig;
    uint32 volid;
    uint8 vollab[11];
    uint8 fstype[8];
} __attribute__((packed));

struct dir_t {
    uint8 name[11];
    uint8 attr;
    uint8 ntres;
    uint8 crttimetenth;
    uint16 crttime;
    uint16 crtdate;
    uint16 lstaccdate;
    uint16 ch;
    uint16 wrttime;
    uint16 wrtdate;
    uint16 cl;
    uint32 size;
} __attribute__((packed));

struct long_dir_t {
    uint8 order;
    uint8 name1[10];
    uint8 attr;
    uint8 type;
    uint8 checksum;
    uint8 name2[12];
    uint16 fstcluslo;
    uint8 name3[4];
} __attribute__((packed));

struct cluster_entry_t{
    union 
    {
        uint32 val;
        struct {
            uint32 idx:28;
            uint32 reserved: 4;
        };
    };  
};

struct filename_t{
    union {
        uint8 fullname[256];
        struct{
            uint8 name[13];
        } part[20];
    };
} __attribute__((packed));


struct fat_file_block_t{
    struct list_head list;
    uint32 oid;
    uint32 cid;
    /* Already read the data into buf*/
    uint32 read;
    uint32 dirty;
    uint8 buf[BLOCK_SIZE];
};

struct fat_file_t{
    /* Head of fat_file_block_t chain */
    struct list_head list;
    uint32 size;
};

struct fat_dir_t {
    /* Head of fat_internal chain */
    struct list_head list;
};

struct fat_info_t{
    struct boot_sector_t bs;
    uint32 fat_lba;
    uint32 cluster_lba;
};

struct fat_internal{
    const char *name;
    struct vnode *node;
    struct fat_info_t *fat;
    struct list_head list;
    uint32 cid;
    uint32 type;
    union {
        struct fat_dir_t *dir;
        struct fat_file_t *file;
    };
};

struct fat_mount_t {
    /* Link fat_mount_t */
    struct list_head list;
    struct mount *mount;
};

int fat32fs_mount(struct filesystem *fs, struct mount *mount);

int fat32fs_lookup(struct vnode *dir_node, struct vnode **target, const char *component_name);
int fat32fs_create(struct vnode *dir_node, struct vnode **target, const char *component_name);
int fat32fs_mkdir(struct vnode *dir_node, struct vnode **target, const char *component_name);
int fat32fs_isdir(struct vnode *dir_node);
int fat32fs_getname(struct vnode *dir_node, const char **name);
int fat32fs_getsize(struct vnode *dir_node);

int fat32fs_write(struct file *file, const void *buf, size_t len);
int fat32fs_read(struct file *file, void *buf, size_t len);
int fat32fs_open(struct vnode *file_node, struct file *target);
int fat32fs_close(struct file *file);
long fat32fs_lseek64(struct file *file, long offset, int whence);
int fat32fs_ioctl(struct file *file, uint64 request, va_list args);
int fat32fs_sync(struct filesystem *fs);
struct filesystem *fat32fs_init(void);

#endif