#ifndef FAT32_H
#define FAT32_H

#include "list.h"

#define BLOCK_SIZE 512
#define CLUSTER_ENTRY_PER_BLOCK (BLOCK_SIZE / sizeof(struct cluster_entry_t))
#define DIR_PER_BLOCK (BLOCK_SIZE / sizeof(struct dir_t))

struct partition_t {
    unsigned char status;
    unsigned char chss_head;
    unsigned char chss_sector;
    unsigned char chss_cylinder;
    unsigned char type;
    unsigned char chse_head;
    unsigned char chse_sector;
    unsigned char chse_cylinder;
    unsigned int lba;
    unsigned int sectors;
} __attribute__((packed));

struct boot_sector_t {
    unsigned char jmpboot[3];
    unsigned char oemname[8];
    unsigned short bytes_per_sector;
    unsigned char sector_per_cluster;
    unsigned short reserved_sector_cnt;
    unsigned char fat_cnt;
    unsigned short root_entry_cnt;
    unsigned short old_sector_cnt;
    unsigned char media;
    unsigned short sector_per_fat16;
    unsigned short sector_per_track;
    unsigned short head_cnt;
    unsigned int hidden_sector_cnt;
    unsigned int sector_cnt;
    unsigned int sector_per_fat32;
    unsigned short extflags;
    unsigned short ver;
    unsigned int root_cluster;
    unsigned short info;
    unsigned short bkbooksec;
    unsigned char reserved[12];
    unsigned char drvnum;
    unsigned char reserved1;
    unsigned char bootsig;
    unsigned int volid;
    unsigned char vollab[11];
    unsigned char fstype[8];
} __attribute__((packed));

// attr of dir_t
#define ATTR_READ_ONLY  0x01
#define ATTR_HIDDEN     0x02
#define ATTR_SYSTEM     0x04
#define ATTR_VOLUME_ID  0x08
#define ATTR_DIRECTORY  0x10
#define ATTR_ARCHIVE    0x20
#define ATTR_FILE_DIR_MASK (ATTR_DIRECTORY | ATTR_ARCHIVE)

struct dir_t {
    unsigned char name[11];
    unsigned char attr;
    unsigned char ntres;
    unsigned char crttimetenth;
    unsigned short crttime;
    unsigned short crtdate;
    unsigned short lstaccdate;
    unsigned short ch;
    unsigned short wrttime;
    unsigned short wrtdate;
    unsigned short cl;
    unsigned int size;
} __attribute__((packed));

struct long_dir_t {
    unsigned char order;
    unsigned char name1[10];
    unsigned char attr;
    unsigned char type;
    unsigned char checksum;
    unsigned char name2[12];
    unsigned short fstcluslo;
    unsigned char name3[4];
} __attribute__((packed));

struct cluster_entry_t {
    union {
        unsigned int val;
        struct {
            unsigned int idx: 28;
            unsigned int reserved: 4;
        };
    };
};

struct filename_t {
    union {
        unsigned char fullname[256];
        struct {
            unsigned char name[13];
        } part[20];
    };
} __attribute__((packed));

struct fat_file_block_t {
    /* Link fat_file_block_t */
    struct list_head list;
    /* 
     * offset id of file
     * offset id N corresponds to offset N * BLOCKS_SIZE of file
     */
    unsigned int oid;
    /* cluster id */
    unsigned int cid;
    unsigned int bufIsUpdated;
    unsigned char buf[BLOCK_SIZE];
};

struct fat_file_t {
    /* Head of fat_file_block_t chain */
    struct list_head list;
    unsigned int size;
};

struct fat_dir_t {
    /* Head of fat_internal chain */
    struct list_head list;
};

struct fat_info_t {
    struct boot_sector_t bs;
    unsigned int fat_lba;
    unsigned int cluster_lba;
};

// type of struct fat_internal
#define FAT_DIR     1
#define FAT_FILE    2

struct fat_internal {
    const char *name;
    struct vnode *node;
    struct fat_info_t *fat;
    /* Link fat_internal */
    struct list_head list;
    /* cluster id */
    unsigned int cid;
    unsigned int type;
    union {
        struct fat_dir_t *dir;
        struct fat_file_t *file;
    };
};

int register_fat32();

#endif
