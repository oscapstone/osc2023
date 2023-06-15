#include "fat32.h"
#include "tmpfs.h"
#include "stdlib.h"

int fat32fs_mounted = 0;

uint32_t FAT1_LBA;      // Logical Block Address, FAT has size of sec_per_fat32*SD_BLOCK_SIZE bytes
uint32_t root_dir_LBA;  // physical block
uint32_t root_cluster;  // boot_sector_t.root_cluster, usually 2
uint32_t sec_per_fat32; // boot_sector_t.sector_per_fat32

filesystem_t fat32fs = {
    .name = "fat32fs",
    .setup_mount = fat32fs_mount};
file_operations_t fat32fs_fops = {.write = fat32fs_write, .read = fat32fs_read, .open = fat32fs_open, .close = fat32fs_close};
vnode_operations_t fat32fs_vops = {.lookup = fat32fs_lookup, .create = fat32fs_create, .mkdir = fat32fs_mkdir};

int fat32fs_mount(struct filesystem *fs, struct mount *mount)
{
    mount->root->f_ops = &fat32fs_fops;
    mount->root->v_ops = &fat32fs_vops;

    /* parse MBR */
    MBR_t mbr;
    readblock(0, &mbr);

    if (mbr.signature[0] != 0x55 || mbr.signature[1] != 0xAA)
        return -1;

    boot_sector_t boot_sector;
    // readblock(mbr.part1.relative_sector, &boot_sector);
    readblock(2048, &boot_sector);

    // FAT1_LBA = mbr.part1.relative_sector + boot_sector.reserved_sector_cnt;
    FAT1_LBA = 2048 + boot_sector.reserved_sector_cnt;
    root_dir_LBA = FAT1_LBA + (boot_sector.fat_cnt * boot_sector.sector_per_fat32);
    root_cluster = boot_sector.root_cluster;
    sec_per_fat32 = boot_sector.sector_per_fat32;

    char *buf = my_malloc(sizeof(char *) * SECTOR_SIZE);
    readblock(root_dir_LBA, buf);
    dir_entry *root_dir_entry = (dir_entry *)buf;

    /* Insert files in the root_dir_entry to the file system */
    vnode_t *dir_node = mount->root;
    vnode_t *node_new = NULL;
    const dir_entry *item = (dir_entry *)root_dir_entry;
    for (int i = 0; i < SD_BLOCK_SIZE / sizeof(dir_entry); i++)
    {
        // Skip empty entry
        if (item[i].name[0] != '\0')
        {
            char *name = my_malloc(sizeof(item->name) + 2); // +1 for . in fileName.ext and end of string

            // Copy entry name
            fat32_filename_to_str(item[i].name, name);

            // Create entry along tmpfs
            switch (tmpfs_lookup(dir_node, &node_new, name))
            {
            case NOTFOUND:
                tmpfs_create(dir_node, &node_new, name);
            }

            // Config entry to a file
            node_new->internal->type = FILE;
            node_new->internal->size = item[i].fileSize;
            node_new->internal->data = (char *)(uint64_t)root_dir_LBA + (item[i].fstClusHI << 16 | item[i].fstClusLO) - root_cluster;
        }
    }

    fat32fs_mounted = 1;
    return 0;
}

int fat32fs_lookup(struct vnode *dir_node, struct vnode **target, char *component_name)
{
    switch (tmpfs_lookup(dir_node, target, component_name))
    {
    case NOTFOUND:
        return NOTFOUND;
    case NOTDIR:
        return NOTDIR;
    case EXISTED:
        return EXISTED;
    }

    return 0;
}

int fat32fs_mkdir(struct vnode *dir_node, struct vnode **target, char *component_name) { return 0; }

int fat32fs_create(struct vnode *dir_node, struct vnode **target, char *component_name)
{
    /* Find free entry in FAT1 and mark it as used (eof) */
    char temp[SD_BLOCK_SIZE];
    uint32_t free_entry_LBA = 0;
    for (int j = 0; j < sec_per_fat32; j++)
    {
        readblock(FAT1_LBA + j, temp);
        uint32_t *fat_entry = (uint32_t *)temp;
        for (int i = 0; i < SD_BLOCK_SIZE / sizeof(uint32_t); i++)
        {
            if (fat_entry[i] == FAT_ENTRY_EMPTY)
            {
                fat_entry[i] = FAT_ENTRY_EOF;   // immediate mark this as entry EOF, i.e., this file occupies 1 sector
                writeblock(FAT1_LBA + j, temp); // write back
                free_entry_LBA = j * SD_BLOCK_SIZE / sizeof(uint32_t) + i;
                break;
            }
        }
        if (free_entry_LBA != 0)
            break;
    }

    if (free_entry_LBA == 0)
    {
        printf("[info/fat32fs_create] Fail to find free entry in FAT1\n");
        return 1;
    }

    /* Update root dir entry */
    readblock(root_dir_LBA, temp);
    dir_entry *root_dir_entry = (dir_entry *)temp;
    dir_entry *item = (dir_entry *)root_dir_entry;
    int i = 0;
    for (i = 0; i < SD_BLOCK_SIZE / sizeof(dir_entry); i++)
    {
        // Find empty entry
        if (item[i].name[0] == '\0')
            break;
    }
    if (i >= SD_BLOCK_SIZE / sizeof(dir_entry))
    {
        printf("[info/fat32fs_write] Fail to find free entry in root_dir_entry\n");
        return -1;
    }

    int j = 0; // component_name[j]
    int k = 0; // item->name[k]
    // Copy filename
    while (k < 8 && component_name[j] != '\0' && component_name[j] != '.')
        item[i].name[k++] = component_name[j++];
    j++; // skip '.'
    // Fill space
    while (k < 8)
        item[i].name[k++] = ' ';
    // Copy extention
    while (k < 11 && component_name[j] != '\0')
        item[i].name[k++] = component_name[j++];

    /* Config entry metadata and write back to SD card */
    item[i].attr = DIR_ENTRY_ATTR_ARCHIVE;
    item[i].fstClusHI = (free_entry_LBA >> 16) & 0x0000FFFF;
    item[i].fstClusLO = free_entry_LBA & 0x0000FFFF;
    item[i].fileSize = 0;
    writeblock(root_dir_LBA, temp); // since item is sort of temp's reference

    readblock(root_dir_LBA, temp);
    root_dir_entry = (dir_entry *)temp;
    item = (dir_entry *)root_dir_entry;
    i = 0;
    for (i = 0; i < SD_BLOCK_SIZE / sizeof(dir_entry); i++)
    {
        // Skip empty entry
        if (item[i].name[0] != '\0')
        {
            char name[13]; // 11 char + '.' + '\0'
            // Copy entry name
            fat32_filename_to_str(item[i].name, name);
        }
    }

    int ret = tmpfs_create(dir_node, target, component_name);
    if (ret == 0)
        (*target)->internal->data = (char *)(uint64_t)root_dir_LBA + free_entry_LBA - root_cluster;
    return ret;
}

// fops
int fat32fs_write(struct file *file, void *buf, size_t len)
{
    len = len <= SD_BLOCK_SIZE ? len : SD_BLOCK_SIZE;
    uint32_t block_cnt = len / SD_BLOCK_SIZE + (len % SD_BLOCK_SIZE != 0);
    char *temp = my_malloc(SD_BLOCK_SIZE * block_cnt);
    char name[13]; // 11 char + '.' + '\0'

    /* Read modify write */
    uint32_t phy_block = (uint64_t)file->vnode->internal->data;
    readblock(phy_block, temp);
    memcpy(temp, buf, len);
    writeblock(phy_block, temp);

    /* Update root dir entry */
    readblock(root_dir_LBA, temp);
    dir_entry *root_dir_entry = (dir_entry *)temp;
    dir_entry *item = (dir_entry *)root_dir_entry;
    int i = 0;
    for (i = 0; i < SD_BLOCK_SIZE / sizeof(dir_entry); i++)
    {
        // Skip empty entry
        if (item[i].name[0] != '\0')
        {
            // Copy entry name
            fat32_filename_to_str(item[i].name, name);
            if (!strcmp(file->vnode->internal->name, name))
                break;
        }
    }
    if (i >= SD_BLOCK_SIZE / sizeof(dir_entry))
    {
        printf("[ERROR/fat32fs_write] %s not found\n", file->vnode->internal->name);
        return -1;
    }
    item[i].fileSize = len;
    writeblock(root_dir_LBA, temp);

    file->vnode->internal->size = len;
    free(temp);

    return len;
}

int fat32fs_read(struct file *file, void *buf, size_t len)
{
    len = len <= file->vnode->internal->size ? len : file->vnode->internal->size;
    uint32_t block_cnt = len / SD_BLOCK_SIZE + (len % SD_BLOCK_SIZE != 0);
    char *temp = my_malloc(SD_BLOCK_SIZE * block_cnt);
    uint32_t phy_block = (uint64_t)file->vnode->internal->data;
    for (int i = 0; i < block_cnt; i++)
        readblock(phy_block + i, temp + i * SD_BLOCK_SIZE);
    memcpy(buf, temp, len);

    free(temp);
    return len;
}

int fat32fs_open(struct vnode *file_node, struct file **target)
{
    return tmpfs_open(file_node, target);
}

int fat32fs_close(struct file *file)
{
    return tmpfs_close(file);
}

int fat32_filename_to_str(const char *fat32_filename, char *str)
{
    // Copy entry name
    int j = 0; // str[j]
    int k = 0; // fat32_filename[k];
    for (k = 0; k < 8 && fat32_filename[k] != ' '; k++)
        str[j++] = fat32_filename[k];
    str[j++] = '.';
    for (k = 8; k < 11 && fat32_filename[k] != ' '; k++)
        str[j++] = fat32_filename[k];
    str[j] = '\0';
    return j;
}