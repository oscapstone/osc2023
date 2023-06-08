#include "fat32.h"
#include "mbr.h"
#include "sdhost.h"
#include "mm.h"
#include "sched.h"
#include "string.h"
#include "mini_uart.h"

struct vnode_operations *fat32_v_ops;
struct file_operations *fat32_f_ops;
int fat32_registered = 0;

static struct fat32_metadata fat32_metadata;

static uint32_t get_cluster_blk_idx(uint32_t cluster_idx) {
    return fat32_metadata.data_region_blk_idx +
           (cluster_idx - fat32_metadata.first_cluster) * fat32_metadata.sector_per_cluster;
}

static uint32_t get_fat_blk_idx(uint32_t cluster_idx) {
    return fat32_metadata.fat_region_blk_idx + (cluster_idx / FAT_ENTRY_PER_BLOCK);
}

int fat32_setup_mount(struct filesystem* fs, struct mount* mount) {

    mount->fs = fs;
    mount->root = fat32_new_node(NULL, "/", DIRECTORY);

    return 0;

}

int fat32_register() {

    if (fat32_registered) return -1;
    fat32_registered = 1;

    fat32_v_ops = (struct vnode_operations *) chunk_alloc(sizeof(struct vnode_operations));
    fat32_f_ops = (struct file_operations *) chunk_alloc(sizeof(struct file_operations));

    fat32_v_ops->lookup = fat32_lookup;
    fat32_v_ops->create = fat32_create;
    fat32_v_ops->mkdir = fat32_mkdir;
    fat32_v_ops->load_vnode = fat32_load_vnode;

    fat32_f_ops->open = fat32_open;
    fat32_f_ops->read = fat32_read;
    fat32_f_ops->write = fat32_write;
    fat32_f_ops->close = fat32_close;

    return 0;

}

struct vnode* fat32_new_node(struct fat32_internal *parent, const char *name, int type) {

    struct vnode *new_node = (struct vnode *)chunk_alloc(sizeof(struct vnode));
    struct fat32_internal *new_internal = (struct fat32_internal *)chunk_alloc(sizeof(struct fat32_internal));

    strcpy(new_internal->name, name);
    new_internal->type = type;
    new_internal->parent = parent;
    new_internal->vnode = new_node;
    new_internal->size = 0;

    if (parent != NULL)
        new_node->parent = parent->vnode;
    new_node->f_ops = fat32_f_ops;
    new_node->v_ops = fat32_v_ops;
    new_node->mount = 0;
    new_node->internal = (void *)new_internal;

    return new_node;

}

int fat32_open(struct vnode *file_node, struct file **target) {
    return SUCCESS;
}

int fat32_close(struct file *file) {
    if (file)
        return SUCCESS;
    else 
        return FAIL;
}

int fat32_write(struct file *file, const void *buf, unsigned len) {
    struct fat32_internal *file_node = (struct fat32_internal *)file->vnode->internal;
    unsigned int f_pos_ori = file->f_pos;
    int fat[FAT_ENTRY_PER_BLOCK];
    char write_buf[BLOCK_SIZE];

    uint32_t current_cluster = file_node->first_cluster;
    int remain_offset = file->f_pos;
    while (remain_offset > 0 && current_cluster >= fat32_metadata.first_cluster && current_cluster != EOC) {
        remain_offset -= BLOCK_SIZE;
        if (remain_offset > 0) {
            readblock(get_fat_blk_idx(current_cluster), fat);
            current_cluster = fat[current_cluster % FAT_ENTRY_PER_BLOCK];
        }
    }

    int buf_idx, f_pos_offset = file->f_pos % BLOCK_SIZE;
    readblock(get_cluster_blk_idx(current_cluster), write_buf);
    for (buf_idx = 0; buf_idx < BLOCK_SIZE - f_pos_offset && buf_idx < len; buf_idx++) {
        write_buf[buf_idx + f_pos_offset] = ((char *)buf)[buf_idx];
    }
    writeblock(get_cluster_blk_idx(current_cluster), write_buf);
    file->f_pos += buf_idx;

    int remain_len = len - buf_idx;
    while (remain_len > 0 && current_cluster >= fat32_metadata.first_cluster && current_cluster != EOC) {
        writeblock(get_cluster_blk_idx(current_cluster), buf + buf_idx);
        file->f_pos += (remain_len < BLOCK_SIZE) ? remain_len : BLOCK_SIZE;
        remain_len -= BLOCK_SIZE;
        buf_idx += BLOCK_SIZE;

        if (remain_len > 0) {
            readblock(get_fat_blk_idx(current_cluster), fat);
            current_cluster = fat[current_cluster % FAT_ENTRY_PER_BLOCK];
        }
    }

    if (file->f_pos > file_node->size) {
        file_node->size = file->f_pos;

        uint8_t sector[BLOCK_SIZE];
        readblock(file_node->dirent_cluster, sector);
        struct fat32_dirent *sector_dirent = (struct fat32_dirent*)sector;

        for (int i=0; sector_dirent[i].name[0] != '\0'; i++) {
            if (sector_dirent[i].name[0] == 0xE5) continue;

            if (((sector_dirent[i].cluster_high << 16) | sector_dirent[i].cluster_low) == file_node->first_cluster) {
                sector_dirent[i].size = (uint32_t)file->f_pos;
            }
        }
        writeblock(file_node->dirent_cluster, sector);
    }

    return file->f_pos - f_pos_ori;
}

int fat32_read(struct file *file, void *buf, unsigned len) {

    struct fat32_internal *file_node = (struct fat32_internal *)file->vnode->internal;
    unsigned int f_pos_ori = file->f_pos;
    uint32_t current_cluster = file_node->first_cluster;
    int remain_len = len;
    int fat[FAT_ENTRY_PER_BLOCK];
    char tmp[512];

    while (remain_len > 0 && current_cluster >= fat32_metadata.first_cluster && current_cluster != EOC) {
        readblock(get_cluster_blk_idx(current_cluster), tmp+file->f_pos);
        for (int i=0; i<512; i++) {
            if (tmp[i] == '\0' || remain_len--<0) break;
            ((char*)buf)[file->f_pos++] = tmp[i];
        }
        
        if (remain_len > 0) {
            readblock(get_fat_blk_idx(current_cluster), fat);
            current_cluster = fat[current_cluster % FAT_ENTRY_PER_BLOCK];
        }
    }

    //printf("[debug] fat32_read buf cont, len: %s %d\n", buf, file->f_pos - f_pos_ori);
    return (file->f_pos - f_pos_ori);
}

int fat32_create(struct vnode *dir_node, struct vnode **target, const char *component_name) {    
    
    uint8_t sector[BLOCK_SIZE];
    struct fat32_internal *dir_internal = (struct fat32_internal *)dir_node->internal;
    readblock(get_cluster_blk_idx(dir_internal->first_cluster), sector);

    struct fat32_dirent *sector_dirent = (struct fat32_dirent *)sector;
    //printf("[debug] 0\n");
    int idx=0;
    while (sector_dirent[idx].name[0] != '\0' && sector_dirent[idx].name[0] != 0xE5) {
        //printf("[debug] first char: 0x%x\n", sector_dirent[idx].name[0]);
        idx++;
    }
    //printf("[debug] 1\n");
    int t=0;
    while (component_name[t] != '.') {
        sector_dirent[idx].name[t] = component_name[t];
        t++;
    }

    int e=t+1;
    while (t < 8) {
        sector_dirent[idx].name[t] = ' ';
        t++;
    }
    while (component_name[e] != '\0') {
        sector_dirent[idx].ext[t-8] = component_name[e];
        e++;
        t++;
    }
    //printf("[debug] 2\n");
    // find empty usable cluster
    int fat[FAT_ENTRY_PER_BLOCK];
    int found = 0;
    uint32_t iter_cluster = fat32_metadata.first_cluster;
    while (found != 1) {
        readblock(get_fat_blk_idx(iter_cluster), fat);
        if (fat[iter_cluster%FAT_ENTRY_PER_BLOCK] == 0x0) {
            //printf("[debug] create at empty cluster %d\n", iter_cluster);
            found = 1;
            sector_dirent[idx].cluster_high = iter_cluster >> 16;
            sector_dirent[idx].cluster_low = iter_cluster & 0xFFFF;
            break;
        }
        iter_cluster++;
    }
    if (found == 0) return FAIL;
    sector_dirent[idx].attr = 0x20;
    sector_dirent[idx].size = 0;
    //sector_dirent[idx].cluster_high = 0; 
    //sector_dirent[idx].cluster_low = 0; 
    writeblock(get_cluster_blk_idx(dir_internal->first_cluster), sector_dirent);

    struct vnode *new_node = fat32_new_node(dir_internal, component_name, REGULAR_FILE);
    struct fat32_internal *new_internal = (struct fat32_internal *)new_node->internal;

    new_internal->dirent_cluster = get_cluster_blk_idx(dir_internal->first_cluster);
    new_internal->first_cluster = ((sector_dirent[idx].cluster_high) << 16) | (sector_dirent[idx].cluster_low);

    dir_internal->child[dir_internal->size] = new_internal;
    dir_internal->size++;

    *target = new_node;
    return SUCCESS;

}

int fat32_mkdir(struct vnode *dir_node, struct vnode **target, const char *component_name) {
    return FAIL;
}

int fat32_lookup(struct vnode *dir_node, struct vnode **target, const char *component_name) {
    //printf("[debug] in fat32 lookup\n");
    if (stringcmp(component_name, "") == 0) {
        *target = dir_node;
        return 0;
    }
    
    struct fat32_internal *internal = (struct fat32_internal *)dir_node->internal;

    for (int i=0; i<internal->size; i++) {
        //printf("[debug] filename: %s\n", internal->child[i]->name);
        if(stringcmp(internal->child[i]->name, component_name) == 0) {
            *target = internal->child[i]->vnode;
            return internal->child[i]->type;
        }
    }
    
    return FAIL;

}

int fat32_load_vnode(struct vnode *dir_node, char *component_name) {
    //printf("[debug] load vnodes\n");
    struct fat32_internal *dir_internal = (struct fat32_internal *)dir_node->internal;
    uint8_t sector[BLOCK_SIZE];
    uint32_t dirent_cluster = get_cluster_blk_idx(dir_internal->first_cluster);
    readblock(dirent_cluster, sector);

    struct fat32_dirent *sector_dirent = (struct fat32_dirent *)sector;

    int found = FAIL;
    for (int i=0; sector_dirent[i].name[0] != '\0'; i++) {

        if (sector_dirent[i].name[0] == 0xE5) continue;
        if (sector_dirent[i].name[0] == 0x2E) continue;

        char filename[13];
        int len=0;
        for (int j=0; j < 8; j++) {
            char c = sector_dirent[i].name[j];
            if (c == ' ') break;
            filename[len++] = c;
        }
        filename[len++] = '.';
        for (int j=0; j < 3; j++) {
            char c = sector_dirent[i].ext[j];
            if (c == ' ') break;
            filename[len++] = c;
        }
        filename[len++] = '\0';

        for (int i=0; i<dir_internal->size; i++) {
            if (stringcmp(filename, dir_internal->child[i]->name) == 0) {
                //printf("[debug] %s already loaded\n", filename);
                continue;
            }
        }

        //printf("[debug] load file: %s\n", filename);
        if (stringcmp(filename, component_name) == 0) found = SUCCESS;

        struct vnode* new_node;
        struct fat32_internal *p_internal = (struct fat32_internal *)dir_node->internal;
        if (sector_dirent[i].attr == 0x10) {
            new_node = fat32_new_node(p_internal, filename, DIRECTORY);

            struct fat32_internal *internal = (struct fat32_internal *)new_node->internal;
            internal->first_cluster = ((sector_dirent[i].cluster_high) << 16) | (sector_dirent[i].cluster_low);
            internal->dirent_cluster = dirent_cluster;

            p_internal->child[p_internal->size] = internal;
        } else {
            new_node = fat32_new_node(p_internal, filename, REGULAR_FILE);
            
            struct fat32_internal *internal = (struct fat32_internal *)new_node->internal;
            internal->first_cluster = ((sector_dirent[i].cluster_high) << 16) | (sector_dirent[i].cluster_low);
            internal->dirent_cluster = dirent_cluster;
            internal->size = sector_dirent[i].size;

            p_internal->child[p_internal->size] = internal;
        }
        
        p_internal->size++;

    }

    return found;

}

void parse_fat32sd() {
    
    char buf[BLOCK_SIZE];
    readblock(0, buf);

    if (buf[510] != 0x55 || buf[511] != 0xAA) return;

    struct mbr_partition_tb pt1;
    for (int i=0; i<sizeof(struct mbr_partition_tb); i++)
        ((char*)&pt1)[i] = ((char*)buf)[446+i];

    readblock(pt1.starting_sector, buf);

    if (pt1.partition_type == 0x0b) {

        vfs_chdir("/boot");
        //printf("[debug] begin parse sd metadata parse.\n");

        struct fat32_boot_sector* boot_sector = (struct fat32_boot_sector*)buf;
        fat32_metadata.data_region_blk_idx = pt1.starting_sector +
                                             boot_sector->n_sectors_per_fat_32 * boot_sector->n_file_alloc_tabs +
                                             boot_sector->n_reserved_sectors;
        fat32_metadata.fat_region_blk_idx = pt1.starting_sector + boot_sector->n_reserved_sectors;
        fat32_metadata.n_fat = boot_sector->n_file_alloc_tabs;
        fat32_metadata.sector_per_fat = boot_sector->n_sectors_per_fat_32;
        fat32_metadata.first_cluster = boot_sector->root_dir_start_cluster_num;
        fat32_metadata.sector_per_cluster = boot_sector->logical_sector_per_cluster;
        
        struct fat32_internal *root_internal = (struct fat32_internal *)current->cwd->internal;
        root_internal->first_cluster = boot_sector->root_dir_start_cluster_num;

        /*
        printf("[debug] fat32_metadata:\n");
        printf("        data_region_blk_idx: 0x%x\n", fat32_metadata.data_region_blk_idx);
        printf("        fat_region_blk_idx: 0x%x\n", fat32_metadata.fat_region_blk_idx);
        printf("        n_fat: %d\n", fat32_metadata.n_fat);
        printf("        sector_per_fat: %d\n", fat32_metadata.sector_per_fat);
        printf("        first_cluster: 0x%x\n", fat32_metadata.first_cluster);
        printf("        sector_per_cluster: %d\n", fat32_metadata.sector_per_cluster);
        */

        vfs_chdir("/");

    }

    return;

}