#ifndef __FAT32_H
#define __FAT32_H
#include "sd.h"
#include "type.h"
#include "fs/vfs.h"
#define ATTR_READ_ONLY 0x01
#define ATTR_HIDDEN 0x02
#define ATTR_SYSTEM 0x04
#define ATTR_VOLUME_ID 0x08
#define ATTR_DIRECTORY 0x10
#define ATTR_ARCHIVE 0x20
#define ATTR_LONG_NAME 0xf /* should be examined first */

#define END_OF_CLUSTER 0x0ffffff8

#define FAT_DIR_CACHE_NUM 16

struct fat32_file_cache {
    char name[16];
    uint32_t filesize;
    uint32_t *FAT;
    uint32_t FAT_cnt;
    // unsigned int root_index; 
    uint32_t dir_ent_clus_idx;
    uint32_t dir_idx;
    void *tmpdata;
    uint32_t data_cluster_index; 
    uint32_t maxsize;
    uint8_t dirty;
    uint8_t cached;
    uint8_t opening;
};

struct fat32_dir_cache {
    char dir_name[16];
    uint32_t *FAT;
    uint32_t dir_clus_idx;
    uint32_t FAT_cnt;
};

struct partition_entry {
    uint8_t state;
    uint8_t beg_head;
    uint16_t beg_cly_sec;
    uint8_t type;
    uint8_t end_head;
    uint16_t end_cly_sec;
    uint32_t num_between;
    uint32_t num_sec;
}__attribute__((__packed__));

struct fat_boot_sector {
    uint8_t jmpcode[3];
    uint8_t oemname[8];
    uint16_t n_bytes_per_sec;
    uint8_t n_sec_per_clus;
    uint16_t n_reserve_sec;
    uint8_t n_copy;
    uint16_t n_root_dir_ent;
    uint16_t tot_sec_not_used;
    uint8_t media_desc;
    uint16_t n_sec_per_fat_not_used;
    uint16_t n_sec_per_track;
    uint16_t n_head;
    uint32_t n_hidden_sec;
    uint32_t tot_sec;
    uint32_t n_sec_per_fat;
    uint16_t mirror_flags;
    uint16_t fs_version;
    uint32_t first_cluster_of_root;
    uint16_t fs_info_reserve;
    uint16_t backup_sec;
    uint8_t reserve[12];
    uint8_t driver_num;
    uint8_t cur_head;
    uint8_t extend_sig;
    uint32_t serial_num;
    uint8_t label[11];
    uint8_t fs_type[8];
}__attribute__((__packed__));

struct fat32_dir_entry {
    uint8_t filename[8];
    uint8_t ext[3];
    uint8_t attr;
    uint8_t reserve[8];
    uint16_t beg_clus_h;
    uint16_t mod_time;
    uint16_t mod_date;
    uint16_t beg_clus_l;
    uint32_t file_size;
}__attribute__((__packed__));
int fat32_init(struct filesystem *fs);
int fat32_setup_mount(struct filesystem *fs, struct mount *mnt);
int fat32_lookup(struct vnode* dir_node, struct vnode **target, const char *component);
int fat32_open(struct vnode* node, struct file **target);
int fat32_read(struct file *f, void *buf, long len);
int fat32_write(struct file *f, void *buf, long len);
int fat32_create(struct vnode *dir_node, struct vnode **target, const char *comp);
void fat32_sync();
int fat32_close(struct file *f);
void setup_boot();
#endif