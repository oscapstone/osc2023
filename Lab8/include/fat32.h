#ifndef _FAT32_H
#define _FAT32_H

#include "vfs.h"
#include "sdhost.h"
#include <stdint.h>

/*
  Ref[1]: https://wiki.osdev.org/MBR_(x86)
  Ref[2]: https://wiki.osdev.org/FAT#Boot_Record
  Ref[3]: https://github.com/LJP-TW/osc2022
  Ref[4]: https://www.easeus.com/resource/fat32-disk-structure.htm
  Ref[5]: https://academy.cba.mit.edu/classes/networking_communications/SD/FAT.pdf
  Ref[6]: https://www.youtube.com/watch?v=lz83GavddB0
  Ref[7]: https://cscie92.dce.harvard.edu/fall2022/slides/FAT32%20File%20Structure.pdf
*/

extern uint32_t FAT1_LBA;      // Logical Block Address, FAT has size of sec_per_fat32*SD_BLOCK_SIZE bytes
extern uint32_t root_dir_LBA;  // physical block
extern uint32_t root_cluster;  // boot_sector_t.root_cluster, usually 2
extern uint32_t sec_per_fat32; // boot_sector_t.sector_per_fat32

#define FROM_LBA_TO_PHY_BK(lba) (root_dir_LBA + (lba) + -root_cluster)
#define FAT_ENTRY_EOF ((uint32_t)0x0FFFFFF8) // end of file (end of linked list in FAT)
#define FAT_ENTRY_EMPTY ((uint32_t)0x00000000)
#define DIR_ENTRY_ATTR_ARCHIVE 0x20   // file, page 23, Ref[5]
#define DIR_ENTRY_wrtTime_mock 0x58D8 // 11:06:48AM
#define DIR_ENTRY_wrtDate_mock 0x50C4 // 20200604

typedef struct partition_struct
{
    uint8_t bootindicator;
    uint8_t start_head;
    uint16_t start_sector_and_cylinder;
    uint8_t fs_type; // '0B':FAT32 ; '04':FAT16 ; '07':NTFS
    uint8_t end_head;
    uint16_t end_sector_and_cylinder;
    uint32_t relative_sector;
    uint32_t number_sectors;
} __attribute__((packed)) partition_t;

typedef struct MBR_struct
{
    uint8_t bootstrap_code[446];
    partition_t partitiontable[4];
    uint8_t signature[2];
} __attribute__((packed)) MBR_t;

// page 7, Ref[5]
typedef struct boot_sector_t
{
    uint8_t jmpboot[3];
    uint8_t oemname[8];
    uint16_t bytes_per_sector;
    uint8_t sector_per_cluster;
    uint16_t reserved_sector_cnt; // Number of reserved sectors in the reserved region of the volume starting at the first sector of the volume. This field is used to align the start of the data area to integral multiples of the cluster size with respect to the start of the partition/media
    uint8_t fat_cnt;              // The count of file allocation tables (FATs) on the volume
    uint16_t root_entry_cnt;
    uint16_t old_sector_cnt;
    uint8_t media;
    uint16_t sector_per_fat16;
    uint16_t sector_per_track;
    uint16_t head_cnt;
    uint32_t hidden_sector_cnt;
    uint32_t sector_cnt;
    uint32_t sector_per_fat32; // This field is the FAT32 32-bit count of sectors occupied by one FAT.
    uint16_t extflags;
    uint16_t ver;
    uint32_t root_cluster;
    uint16_t info; // Sector number of FSINFO structure in the reserved area of the FAT32 volume. Usually 1.
    uint16_t bkbooksec;
    uint8_t reserved[12];
    uint8_t drvnum;
    uint8_t reserved1;
    uint8_t bootsig;
    uint32_t volid;
    uint8_t vollab[11];
    uint8_t fstype[8];
    uint8_t skipped[420];
    uint8_t valid_bootsector[2]; // 0x55, 0xAA
} __attribute__((packed)) boot_sector_t;

// page 23, Ref[5]
typedef struct dir_entry
{
    char name[11];        // 0~10
    uint8_t attr;         // 11
    uint8_t NTRes;        // 12
    uint8_t crtTimeTenth; // 13
    uint16_t crtTime;     // 14~15
    uint16_t crtDate;     // 16~17
    uint16_t lstAccDate;  // 18~19
    uint16_t fstClusHI;   // 20~21, High word of first data cluster number for file/directory described by this entry.
    uint16_t wrtTime;     // 22~23
    uint16_t wrtDate;     // 24~26
    uint16_t fstClusLO;   // 26~27, Low word of first data cluster number for file/directory described by this entry.
    uint32_t fileSize;    // 28~31, 32-bit quantity containing size in bytes of file/directory described by this entry.
} __attribute__((packed)) dir_entry;

// VFS bridge -----------------------------------------------------------
extern int fat32fs_mounted;
int fat32fs_mount(struct filesystem *fs, struct mount *mount);

// fops
int fat32fs_write(struct file *file, void *buf, size_t len); // no lseek()
int fat32fs_read(struct file *file, void *buf, size_t len);  // no lseek()
int fat32fs_open(struct vnode *file_node, struct file **target);
int fat32fs_close(struct file *file);

// vops
int fat32fs_mkdir(struct vnode *dir_node, struct vnode **target, char *component_name);
int fat32fs_create(struct vnode *dir_node, struct vnode **target, char *component_name);
int fat32fs_lookup(struct vnode *dir_node, struct vnode **target, char *component_name);

int fat32_filename_to_str(const char *fat32_filename, char *str);

#endif