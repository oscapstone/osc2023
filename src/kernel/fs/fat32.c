#include "fs/fat32.h"
#include "sd.h"
#include "fs/vfs.h"
#include "mem/mem.h"
#include "mem/page.h"
#include "peripherals/mini_uart.h"
#include "utils.h"


struct partition_entry pe[4];
unsigned int fat_base_idx;
unsigned int fat_blk_idx;
unsigned int fat_data_region_idx;

struct filesystem fat32_fs = {
    .name = "fat32_boot",
    .init_fs = &fat32_init,
    .root = NULL,
    .setup_mount = &fat32_setup_mount
};

struct file_operations fat32_fops = {
    .close = fat32_close,
    .ioctl = fops_not_support,
    .lseek64 = fops_not_support,
    .open = fat32_open,
    .read = fat32_read,
    .write = fat32_write,
    .stat = fops_not_support
};
struct vnode_operations fat32_vops = {
    .create = fat32_create,
    .mkdir = vops_not_support,
    .lookup = fat32_lookup
};

int end_of_cluster(unsigned int val) {
    if(val >= END_OF_CLUSTER) {
        return 1;
    } else {
        return 0;        
    }
}

int fat32_close(struct file *f) {
    struct fat32_file_cache *f_cache = f->vnode->internal;
    f_cache->opening -= 1;
    kfree(f);
    return 0;
}
int dirent2filename(struct fat32_dir_entry *ent, char *buf) {
    int ridx = 7;
    for(;ridx >= 0; ridx --) {
        if(ent->filename[ridx] != ' ') {
            break;
        }
    }
    if(ridx < 0) return 0;
    memcpy(buf, ent->filename, ridx + 1);
    int ext_ridx = 2;
    for(; ext_ridx >= 0; ext_ridx --) {
        if(ent->ext[ext_ridx] != ' ') {
            break;            
        }
    }

    if(ext_ridx >= 0) {
        buf[ridx + 1] = '.';
        memcpy(buf + ridx + 2, ent->ext, ext_ridx + 1);
        buf[ridx + 2 + ext_ridx + 1] = '\0';
        return ridx + 2 + ext_ridx + 1;
    } else {
        buf[ridx + 1] = '\0';
        return ridx + 1;
    }
}

int clusteridx2datablkidx(int clus_idx) {
    return clus_idx - 2 + fat_data_region_idx;
}

int build_up_fat(uint8_t **FAT, int beg_clus_idx) {
    int cnt = 0;
    int cur_clus_idx = beg_clus_idx;
    int old_clus_idx = cur_clus_idx - (cur_clus_idx & 0x7f);
    unsigned int buf[150];
    readblock(fat_blk_idx + (old_clus_idx >> 7),&(buf[0]));
    while(!end_of_cluster(cur_clus_idx)) {
        if(cur_clus_idx - old_clus_idx >= 0x80 || cur_clus_idx - old_clus_idx <= -0x80) {
            old_clus_idx = cur_clus_idx - (cur_clus_idx & 0x7f);
            readblock(fat_blk_idx + (old_clus_idx >> 7), &(buf[0]));
        }
        cur_clus_idx = buf[cur_clus_idx % 128];
        cnt += 1;
    }
    uint32_t *new_FAT = (unsigned int*)kmalloc(cnt * 4);

    *FAT = new_FAT;
    cur_clus_idx = beg_clus_idx;
    old_clus_idx = cur_clus_idx - (cur_clus_idx & 0x7f);
    cnt = 0;
    readblock(fat_blk_idx + (old_clus_idx >> 7),&(buf[0]));
    while(!end_of_cluster(cur_clus_idx)) {

        if(cur_clus_idx - old_clus_idx >= 0x80 || cur_clus_idx - old_clus_idx <= -0x80) {
            old_clus_idx = cur_clus_idx - (cur_clus_idx & 0x7f);
            readblock(fat_blk_idx + (old_clus_idx >> 7), &(buf[0]));
        }
        cur_clus_idx = buf[cur_clus_idx % 128];
        new_FAT[cnt++] = cur_clus_idx;
    }
    return cnt;
}

void init_fat32_dir_vnode(struct vnode *node, struct fat32_dir_entry *ent, const char *comp) {
    node->internal = kmalloc(sizeof(struct fat32_dir_cache));
    node->flag = VFS_DIR;
    node->f_ops = &fat32_fops;
    node->v_ops = &fat32_vops;
    node->mount = NULL;
    node->parent = NULL;
    ds_list_head_init(&(node->ch_list));
    ds_list_head_init(&(node->v_head));
    struct fat32_dir_cache *dir_cache = node->internal;
    if(comp != NULL) {
        memcpy(dir_cache->dir_name, comp, strlen(comp));
    }

    dir_cache->dir_clus_idx = ent->beg_clus_l - 2;

    dir_cache->FAT_cnt = build_up_fat(&(dir_cache->FAT), dir_cache->dir_clus_idx);
}

void init_fat32_file_vnode(struct vnode* node, struct fat32_dir_entry *ent, const char *comp) {
    node->internal = kmalloc(sizeof(struct fat32_file_cache));
    node->flag = VFS_FILE;
    node->f_ops = &fat32_fops;
    node->v_ops = &fat32_vops;
    node->mount = NULL;
    node->parent = NULL;
    ds_list_head_init(&(node->ch_list));
    ds_list_head_init(&(node->v_head));
    struct fat32_file_cache *file_cache = node->internal;
    if(comp != NULL) {
        memcpy(file_cache->name, comp, strlen(comp));
    }
    file_cache->filesize = ent->file_size;
    file_cache->data_cluster_index = ent->beg_clus_l;

    int cur_clus_idx = file_cache->data_cluster_index;
    file_cache->dirty = 0;
    file_cache->cached = 0;
    file_cache->opening = 0;

    file_cache->FAT_cnt = build_up_fat(&(file_cache->FAT), file_cache->data_cluster_index);

}


int fat32_lookup(struct vnode* dir_node, struct vnode **target, const char *component) {
    struct fat32_dir_cache *dir_cache = dir_node->internal;


    struct ds_list_head *head = dir_node->ch_list.next;
    while(head != &(dir_node->ch_list)) {
        struct fat32_file_cache *f_cache;
        struct vnode *node = container_of(head, struct vnode, v_head);
        f_cache = node->internal;
        if(strncmp(component, f_cache->name, strlen(component)) == 0) {
            *target = node;
            return 0;
        }
        head = ds_list_front(head);
    }

    char filename[12];

    // if not found
    char buf[768];
    readblock(clusteridx2datablkidx(dir_cache->dir_clus_idx), buf);
    int cur_clus_idx = dir_cache->dir_clus_idx;
    struct fat32_dir_entry *ent = buf;
    int fat_idx = 0;
    while(!end_of_cluster(cur_clus_idx)) {

        for(int i = 0; i < 16; i ++) {
            int len = dirent2filename(ent, filename);
            if(strncmp(filename, component, strlen(component)) == 0){
                struct vnode *cur_node = (struct vnode*)kmalloc(sizeof(struct vnode));
                if(ent->attr == ATTR_DIRECTORY) {
                    init_fat32_dir_vnode(cur_node, ent, component);
                    cur_node->parent = dir_node;
                } else {
                    init_fat32_file_vnode(cur_node, ent, component);
                    struct fat32_file_cache *f_cache = cur_node->internal;
                    f_cache->dir_ent_clus_idx = cur_clus_idx;
                    f_cache->dir_idx = i;
                    cur_node->parent = dir_node;
                }

                ds_list_addprev(&(dir_node->ch_list), &(cur_node->v_head));
                *target = cur_node;
                return 0;
            }
            ent = (buf + 32 * i);
        }
        cur_clus_idx = dir_cache->FAT[fat_idx ++];
    }
    return VFS_NOT_FOUND;
}

int fat32_init(struct filesystem *fs) {
    fs->root = (struct vnode*)kmalloc(sizeof(struct vnode));

    ds_list_head_init(&(fs->root->ch_list));    
    ds_list_head_init(&(fs->root->v_head));
    fs->root->flag = VFS_DIR;
    fs->root->v_ops = &fat32_vops;
    fs->root->f_ops = &fat32_fops;
    fs->root->internal = (struct fat32_dir_cache*)kmalloc(sizeof(struct fat32_dir_cache));
    struct fat32_dir_cache *dir_cache = fs->root->internal;
    // dir_cache->dir_clus_idx = fat_blk_idx + fat_base_idx;
    dir_cache->dir_clus_idx = 2;
    // int cur_clus_idx = dir_cache->dir_clus_idx;
    // int old_clus_idx = cur_clus_idx - (cur_clus_idx & 0x7f);

    dir_cache->FAT_cnt = build_up_fat(&(dir_cache->FAT), dir_cache->dir_clus_idx);
    return 0;
}

int fat32_open(struct vnode* node, struct file **target) {
    if(node->flag == VFS_DIR) return VFS_CANNOT_OPEN_DIR;
    struct file *file = (struct file*)kmalloc(sizeof(struct file));
    struct fat32_file_cache *file_cache = node->internal;
    file_cache->opening += 1;

    file->f_ops = &fat32_fops;
    file->f_pos = 0;
    file->vnode = node;

    *target = file;
    return 0;
}

uint32_t bin_align(uint32_t val) {
    int ret = 4096;
    while(ret < val) {
        ret *= 2;
    }
    return ret;
}
void build_up_cache(struct fat32_file_cache *file_cache) {
    file_cache->maxsize = bin_align(file_cache->filesize);
    file_cache->tmpdata = cmalloc(file_cache->maxsize);
    int cur_clus_idx = file_cache->data_cluster_index;
    int cnt = 0;

    while(!end_of_cluster(cur_clus_idx)) {
        readblock(clusteridx2datablkidx(cur_clus_idx), file_cache->tmpdata + cnt * SECTOR_SIZE);
        cur_clus_idx = file_cache->FAT[cnt ++];
    }
}

int fat32_read(struct file *f, void *buf, long len) {
    struct vnode* node = f->vnode;

    struct fat32_file_cache *file_cache = node->internal;
    if(file_cache->cached == 0) {
        // load the file into cache
        // fully cache for each file
        // maybe not very good implementation
        build_up_cache(file_cache);
        file_cache->cached = 1;
    }

    
    for(int i = 0; i < len; i ++, f->f_pos ++) {
        if(f->f_pos == file_cache->filesize) {
            return i;
        }
        *(char *)(buf + i) = *(char *)(file_cache->tmpdata + f->f_pos);
    }

    return len;
}

int fat32_write(struct file *f, void *buf, long len) {
    struct vnode* node = f->vnode;

    struct fat32_file_cache *file_cache = node->internal;
    if(file_cache->cached == 0) {
        // load the file into cache
        // fully cache for each file
        // maybe not very good implementation
        build_up_cache(file_cache);
        file_cache->cached = 1;
    }
    file_cache->dirty = 1;
    
    for(int i = 0; i < len; i ++, f->f_pos ++) {
        if(f->f_pos == file_cache->maxsize) {
            // if hit maxsize, alloc new cache and free old one
            void *newcache = cmalloc(file_cache->maxsize << 1);
            memcpy(newcache, file_cache->tmpdata, file_cache->maxsize);
            kfree(file_cache->tmpdata);
            file_cache->tmpdata = newcache;
            file_cache->maxsize <<= 1;
        }
        if(f->f_pos >= file_cache->filesize) {
            file_cache->filesize = f->f_pos + 1;
        }
        *(char *)(file_cache->tmpdata + f->f_pos) = *(char *)(buf + i);
    }

    return len;
}

int fat32_create(struct vnode *dir_node, struct vnode **target, const char *comp) {
    // uart_send_string("In fat32 create\r\n");
    struct vnode *new_node = (struct vnode*)kmalloc(sizeof(struct vnode));

    new_node->internal = kmalloc(sizeof(struct fat32_file_cache));
    new_node->flag = VFS_FILE;
    new_node->f_ops = &fat32_fops;
    new_node->v_ops = &fat32_vops;
    new_node->mount = NULL;
    new_node->parent = NULL;
    ds_list_head_init(&(new_node->ch_list));
    ds_list_head_init(&(new_node->v_head));
    struct fat32_file_cache *file_cache = new_node->internal;
    if(comp != NULL) {
        memcpy(file_cache->name, comp, strlen(comp));
    }
    file_cache->name[strlen(comp)] = '\0';
    file_cache->filesize = 0;

    int cur_clus_idx = file_cache->data_cluster_index;
    file_cache->dirty = 1;
    file_cache->cached = 1;
    file_cache->opening = 0;
    file_cache->tmpdata = cmalloc(PAGE_SIZE);
    file_cache->maxsize = PAGE_SIZE;
    file_cache->filesize = 1;
    struct fat32_dir_cache *dir_cache = dir_node->internal;
    file_cache->dir_ent_clus_idx = dir_cache->dir_clus_idx;
    file_cache->dir_idx = 0xffffffff;
    file_cache->FAT_cnt = 0xffffffff;
    file_cache->FAT = NULL;

    ds_list_addprev(&(dir_node->ch_list), &(new_node->v_head));
    new_node->parent = dir_node;

    *target = new_node;

    return 0;
}

int fat32_setup_mount(struct filesystem *fs, struct mount *mnt) {
    // uart_send_string("In fat32 setup mount\r\n");
    mnt->root = fs->root;
    mnt->fs = fs;
    if(fs->root->mount == NULL) {
        fs->root->mount = mnt;
    }
    return 0;
}

void setup_boot() {
    char buf[1024];
    readblock(0, buf);
    for(int i = 0; i < 4; i ++) {
      memcpy(&pe[i], buf + (446 + 16 * i), 16);
    }

    // read first partition
    struct fat_boot_sector partition1;

    readblock(pe[0].num_between, buf);
    fat_base_idx = pe[0].num_between;
    memcpy(&partition1, buf, sizeof(struct fat_boot_sector));

    fat_blk_idx = partition1.n_reserve_sec + fat_base_idx;
    fat_data_region_idx = partition1.n_copy * partition1.n_sec_per_fat + partition1.n_reserve_sec + fat_base_idx;

    register_filesystem(&fat32_fs);
    vfs_mkdir("/boot");
    vfs_mount("/boot", "fat32_boot");
}

void fat32_sync_dir(struct vnode* dir_node) {
    struct ds_list_head *head = dir_node->ch_list.next;
    while(head != &(dir_node->ch_list)) {
        struct vnode *node = container_of(head, struct vnode, v_head);
        if(node->flag == VFS_FILE) {
            fat32_sync_file(node, dir_node);
        } else {
            fat32_sync_dir(node);
        }
        head = ds_list_front(head);
    }
}

void gen_new_fat(struct fat32_file_cache *f_cache, uint32_t new_fat_cnt) {
    uint32_t *new_fat = kmalloc(new_fat_cnt * 4);
    int cnt = 0;
    if(f_cache->FAT_cnt != 0xffffffff) {
        for(; cnt < f_cache->FAT_cnt - 1; cnt ++) {
            new_fat[cnt] = f_cache->FAT[cnt];
        }
    } else {
        cnt = -1;
    }



    uint32_t buf[150];
    int idx = 0;

    while(cnt < (int)(new_fat_cnt - 1)) {
        readblock(idx + fat_blk_idx, &(buf[0]));
        for(int i = 0; i < 128; i ++) {
            if(buf[i] == 0) {
                if(f_cache->FAT_cnt == 0xffffffff) {
                    f_cache->data_cluster_index = idx * 128 + i;
                    f_cache->FAT_cnt = new_fat_cnt;
                    cnt ++;
                } else {
                    new_fat[cnt] = idx * 128 + i;
                    cnt ++;
                }
                if(cnt >= new_fat_cnt - 1) break;
            }
        }
        idx ++;
    }
    new_fat[new_fat_cnt - 1] = END_OF_CLUSTER;
    kfree(f_cache->FAT);
    f_cache->FAT = new_fat;
    f_cache->FAT_cnt = new_fat_cnt;
}

void flush_new_fat(struct fat32_file_cache *f_cache) {
    uint32_t cur_clus_idx = f_cache->data_cluster_index;
    uint32_t buf[150];
    int cnt = 0;
    while(!end_of_cluster(cur_clus_idx)) {
        readblock((cur_clus_idx >> 7) + fat_blk_idx, &(buf[0]));
        buf[cur_clus_idx & 0x7f] = f_cache->FAT[cnt];
        writeblock((cur_clus_idx >> 7) + fat_blk_idx, &(buf[0]));
        cur_clus_idx = f_cache->FAT[cnt];
        cnt += 1;
    }
}
void flush_data(struct fat32_file_cache *f_cache) {
    uint32_t cur_clus_idx = f_cache->data_cluster_index;
    int cnt = 0;
    while(!end_of_cluster(cur_clus_idx)) {
        writeblock(clusteridx2datablkidx(cur_clus_idx), (f_cache->tmpdata + 512 * cnt));
        cur_clus_idx = f_cache->FAT[cnt];
        cnt += 1;
    }
}

void copy_name(struct fat32_dir_entry *ent, struct fat32_file_cache *f_cache) {
    uint8_t found = 0;
    int i;
    for(i = 0; i < 8; i ++) {
        if(f_cache->name[i] == '.') {
            found = i;
        }
        if(found) {
            ent->filename[i] = ' ';
        } else {
            if(f_cache->name[i] >= 'a' && f_cache->name[i] <= 'z') {
                ent->filename[i] = f_cache->name[i] + ('A' - 'a');
            } else {
                ent->filename[i] = f_cache->name[i];
            }
        }
    }
    if(found == 0) return;
    for(i = found + 1; i <= found + 3; i ++) {
        if(f_cache->name[i] == '\0') {
            break;
        }
        if(f_cache->name[i] >= 'a' && f_cache->name[i] <= 'z') {
            ent->ext[i - found - 1] = f_cache->name[i] + ('A' - 'a');
        } else {
            ent->ext[i - found - 1] = f_cache->name[i];
        }
    }
}

void fat32_sync_file(struct vnode* file_node, struct vnode *dir_node) {
    
    struct fat32_file_cache *f_cache = file_node->internal;
    if(!f_cache->dirty) return;

    char buf[768];
    if(f_cache->dir_idx != 0xffffffff) {
        // update the dir entry for the file
        readblock(clusteridx2datablkidx(f_cache->dir_ent_clus_idx), buf);
        struct fat32_dir_entry *ent = ((void*)(buf + 32 * f_cache->dir_idx));
        ent->file_size = f_cache->filesize;
        writeblock(clusteridx2datablkidx(f_cache->dir_ent_clus_idx), buf);

        // update the file entry for the file
        uint32_t new_fat_cnt = (f_cache->filesize + SECTOR_SIZE - 1) / SECTOR_SIZE;
        if(f_cache->FAT_cnt < new_fat_cnt) {
            gen_new_fat(f_cache, new_fat_cnt);
            flush_new_fat(f_cache);
        }
        flush_data(f_cache);
    } else {
        uint32_t new_fat_cnt = (f_cache->filesize + SECTOR_SIZE - 1) / SECTOR_SIZE;
        gen_new_fat(f_cache, new_fat_cnt);
        flush_new_fat(f_cache);
        int cur_clus_idx = f_cache->dir_ent_clus_idx;
        struct fat32_dir_cache *dir_cache = dir_node->internal;
        int cnt = 0;
        while(!end_of_cluster(cur_clus_idx)) {
            readblock(clusteridx2datablkidx(cur_clus_idx), buf);
            struct fat32_dir_entry *ent;
            for(int i = 0; i < 16; i ++) {
                ent = ((void*)(buf + 32 * i));
                if(*(uint64_t*)(ent) == 0) {
                    ent->attr = ATTR_ARCHIVE;
                    ent->beg_clus_l = f_cache->data_cluster_index & 0xffff;
                    ent->file_size = f_cache->filesize;
                    copy_name(ent, f_cache);
                    writeblock(clusteridx2datablkidx(cur_clus_idx), buf);
                    goto find;
                }
            }
            cur_clus_idx = dir_cache->FAT[cnt ++];
        }

        struct fat32_file_cache tmp;
        tmp.FAT_cnt = dir_cache->FAT_cnt;
        tmp.FAT = dir_cache->FAT;
        tmp.data_cluster_index = dir_cache->dir_clus_idx;

        // if not found, find a new fat table
        // this implementation is not very good
        // but I don't wanna spend more time on this
        gen_new_fat(&(tmp), dir_cache->FAT_cnt + 1);
        dir_cache->FAT_cnt = tmp.FAT_cnt;
        dir_cache->FAT = tmp.FAT;
        flush_new_fat(&(tmp));
        cur_clus_idx = tmp.FAT[tmp.FAT_cnt - 2];
        readblock(clusteridx2datablkidx(cur_clus_idx), buf);

        struct fat32_dir_entry *ent;
        ent = buf;
        ent->attr = ATTR_ARCHIVE;
        ent->beg_clus_l = f_cache->data_cluster_index & 0xffff;
        ent->beg_clus_h = (f_cache->data_cluster_index >> 16) & 0xffff;
        ent->file_size = f_cache->filesize;
        copy_name(ent, f_cache);
        writeblock(clusteridx2datablkidx(cur_clus_idx), buf);

        find:;
        flush_data(f_cache);
    }

}

void fat32_sync() {
    fat32_sync_dir(fat32_fs.root);
}