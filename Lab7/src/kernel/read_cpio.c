#include "stdlib.h"
#include "mini_uart.h"
#include "vfs.h"
#include "tmpfs.h"
#include "read_cpio.h"

extern char *cpioDestGlobal;
cpio_node_t *cpio_list;

typedef struct cpio_newc_header
{
    char c_magic[6];
    char c_ino[8];
    char c_mode[8];
    char c_uid[8];
    char c_gid[8];
    char c_nlink[8];
    char c_mtime[8];
    char c_filesize[8];
    char c_devmajor[8];
    char c_devminor[8];
    char c_rdevmajor[8];
    char c_rdevminor[8];
    char c_namesize[8];
    char c_check[8];
} __attribute__((packed)) cpio_t;

void read_cpio(char *cpioDest)
{
    uart_send_string("Type     Offset   Size     Access rights\tFilename\n");

    while (!memcmp(cpioDest, "070701", 6) && memcmp(cpioDest + sizeof(cpio_t), "TRAILER!!!", 10))
    {
        cpio_t *header = (cpio_t *)cpioDest;
        int ns = hex2int(header->c_namesize, 8);
        int fs = hex2int(header->c_filesize, 8);
        // print out meta information
        uart_hex(hex2int(header->c_mode, 8)); // mode (access rights + type)
        uart_send(' ');
        uart_hex((unsigned int)((unsigned long)cpioDest) + sizeof(cpio_t) + ns);
        uart_send(' ');
        uart_hex(fs); // file size in hex
        uart_send(' ');
        uart_hex(hex2int(header->c_uid, 8)); // user id in hex
        uart_send('.');
        uart_hex(hex2int(header->c_gid, 8)); // group id in hex
        uart_send('\t');
        uart_send_string(cpioDest + sizeof(cpio_t)); // filename
        uart_send_string("\n");
        // jump to the next file
        if (fs % 4 != 0)
            fs += 4 - fs % 4;
        if ((sizeof(cpio_t) + ns) % 4 != 0)
            cpioDest += (sizeof(cpio_t) + ns + (4 - (sizeof(cpio_t) + ns) % 4) + fs);
        else
            cpioDest += (sizeof(cpio_t) + ns + ((sizeof(cpio_t) + ns) % 4) + fs);
    }
}

void read_content(char *cpioDest, char *filename)
{
    int flag = 0;
    while (!memcmp(cpioDest, "070701", 6) && memcmp(cpioDest + sizeof(cpio_t), "TRAILER!!!", 10))
    {
        cpio_t *header = (cpio_t *)cpioDest;
        int ns = hex2int(header->c_namesize, 8);
        // Check filename
        if (!memcmp(cpioDest + sizeof(cpio_t), filename, ns - 1))
        {
            flag = 1;
            break;
        }
        int fs = hex2int(header->c_filesize, 8);
        // jump to the next file
        if (fs % 4 != 0)
            fs += 4 - fs % 4;
        if ((sizeof(cpio_t) + ns) % 4 != 0)
            cpioDest += (sizeof(cpio_t) + ns + (4 - (sizeof(cpio_t) + ns) % 4) + fs);
        else
            cpioDest += (sizeof(cpio_t) + ns + ((sizeof(cpio_t) + ns) % 4) + fs);
    }
    // No hit
    if (flag == 0)
    {
        printf("cat: %s: No such file\n", filename);
        return;
    }
    // Found target file
    cpio_t *header = (cpio_t *)cpioDest;
    int ns = hex2int(header->c_namesize, 8);
    int fs = hex2int(header->c_filesize, 8);
    if ((sizeof(cpio_t) + ns) % 4 != 0)
        cpioDest += (sizeof(cpio_t) + ns + (4 - (sizeof(cpio_t) + ns) % 4));
    else
        cpioDest += (sizeof(cpio_t) + ns + ((sizeof(cpio_t) + ns) % 4));

    // print content
    uart_send_string_of_size((char *)cpioDest, fs);
}

char *find_content_addr(char *cpioDest, const char *filename)
{
    int flag = 0;
    while (!memcmp(cpioDest, "070701", 6) && memcmp(cpioDest + sizeof(cpio_t), "TRAILER!!!", 10))
    {
        cpio_t *header = (cpio_t *)cpioDest;
        int ns = hex2int(header->c_namesize, 8);
        // Check filename
        if (!memcmp(cpioDest + sizeof(cpio_t), (char *)filename, ns - 1))
        {
            flag = 1;
            break;
        }
        int fs = hex2int(header->c_filesize, 8);
        // jump to the next file
        if (fs % 4 != 0)
            fs += 4 - fs % 4;
        if ((sizeof(cpio_t) + ns) % 4 != 0)
            cpioDest += (sizeof(cpio_t) + ns + (4 - (sizeof(cpio_t) + ns) % 4) + fs);
        else
            cpioDest += (sizeof(cpio_t) + ns + ((sizeof(cpio_t) + ns) % 4) + fs);
    }
    // No hit
    if (flag == 0)
    {
        printf("[ERROR] find_content_addr: %s: No such file\n", filename);
        return NULL;
    }

    return cpioDest;
}

int load_userprogram(const char *filename, char *userDest)
{
    char *cpioUserPgmDest = cpioDestGlobal;
    cpioUserPgmDest = find_content_addr(cpioUserPgmDest, filename);
    if (cpioUserPgmDest == NULL)
    {
        printf("[ERROR] FAIL to find %s\n", filename);
        return -1;
    }

    // Found target file
    cpio_t *header = (cpio_t *)cpioUserPgmDest;
    int ns = hex2int(header->c_namesize, 8);
    int fs = hex2int(header->c_filesize, 8);
    if ((sizeof(cpio_t) + ns) % 4 != 0)
        cpioUserPgmDest += (sizeof(cpio_t) + ns + (4 - (sizeof(cpio_t) + ns) % 4));
    else
        cpioUserPgmDest += (sizeof(cpio_t) + ns + ((sizeof(cpio_t) + ns) % 4));

    printf("load %p to %p\n", cpioUserPgmDest, userDest);
    printf("size: %d bytes\n", fs);

    // load content
    while (fs--)
    {
        *userDest++ = *cpioUserPgmDest++;
    }

    if (fs == -1)
        return 0;

    return 1;
}

/* For Lab7 */
int get_size(cpio_t *root_addr, char *attr)
{
    char *temp_addr = (char *)root_addr;

    if (!strcmp(attr, "name"))
        temp_addr += 94;
    else if (!strcmp(attr, "file"))
        temp_addr += 54;

    char size_string[9];
    for (int i = 0; i < 8; i++)
    {
        size_string[i] = temp_addr[i];
    }

    size_string[8] = '\0';

    // hexadecimal to decimal
    return hex2int(size_string, 8);
}

int initramfs_mount(filesystem_t *fs, mount_t *mount)
{
    mount->root->v_ops = my_malloc(sizeof(vnode_operations_t));
    mount->root->v_ops->lookup = initramfs_lookup;
    mount->root->v_ops->create = initramfs_create;
    mount->root->v_ops->mkdir = initramfs_mkdir;

    mount->root->f_ops = my_malloc(sizeof(file_operations_t));
    mount->root->f_ops->write = initramfs_write;
    mount->root->f_ops->read = initramfs_read;
    mount->root->f_ops->open = initramfs_open;
    mount->root->f_ops->close = initramfs_close;

    init_cpio();

    cpio_node_t *tmp = cpio_list;
    int i = 0;
    while (tmp != NULL)
    {
        mount->root->internal->entry[i] = my_malloc(sizeof(vnode_t));
        mount->root->internal->entry[i]->mount = NULL;
        mount->root->internal->entry[i]->v_ops = mount->root->v_ops;
        mount->root->internal->entry[i]->f_ops = mount->root->f_ops;

        mount->root->internal->entry[i]->internal = my_malloc(sizeof(node_info_t));
        mount->root->internal->entry[i]->internal->name = my_malloc(COMPONENT_NAME_MAX);
        strcpy(mount->root->internal->entry[i]->internal->name, tmp->name);
        mount->root->internal->entry[i]->internal->type = tmp->type;
        mount->root->internal->entry[i]->internal->size = tmp->size;
        mount->root->internal->entry[i]->internal->data = tmp->data;

        mount->root->internal->size++;

        tmp = tmp->next;
        i++;
    }

    return 0;
}

void init_cpio()
{
    cpio_t *current_file = (cpio_t *)cpioDestGlobal;

    int namesize = get_size(current_file, "name") - 1;
    int filesize = get_size(current_file, "file");

    char *temp_addr = (char *)(current_file + 1);

    char temp_name[30];

    for (int i = 0; i < namesize; i++)
    {
        temp_name[i] = temp_addr[i];
    }

    temp_name[namesize] = '\0';

    if (!strcmp(temp_name, "TRAILER!!!"))
        return;

    cpio_list = my_malloc(sizeof(cpio_node_t));
    cpio_node_t *tmp = cpio_list;

    while (strcmp(temp_name, "TRAILER!!!"))
    {
        tmp->name = my_malloc(namesize + 10);
        strcpy(tmp->name, temp_name);
        tmp->type = FILE;
        tmp->size = filesize;

        int NUL_nums = 1;
        char *next_file = (char *)(current_file + 1);

        while ((2 + namesize + NUL_nums) % 4 != 0)
            NUL_nums++;

        next_file += (namesize + NUL_nums);
        tmp->data = next_file;

        NUL_nums = 0;
        while ((filesize + NUL_nums) % 4 != 0)
            NUL_nums++;

        next_file += (filesize + NUL_nums);
        current_file = (cpio_t *)next_file;

        namesize = get_size(current_file, "name") - 1;
        filesize = get_size(current_file, "file");

        temp_addr = (char *)(current_file + 1);

        for (int i = 0; i < namesize; i++)
            temp_name[i] = temp_addr[i];

        temp_name[namesize] = '\0';

        if (strcmp(temp_name, "TRAILER!!!"))
        {
            tmp->next = my_malloc(sizeof(cpio_node_t));
            tmp = tmp->next;
        }
        else
            tmp->next = NULL;
    }
}

/* vnode operations : defined in tmpfs.h not in cpio.h */
int initramfs_lookup(struct vnode *dir_node, struct vnode **target, char *component_name)
{
    return tmpfs_lookup(dir_node, target, component_name);
}

int initramfs_create(struct vnode *dir_node, struct vnode **target, char *component_name)
{
    printf("/initramfs is read-only!!\n");
    return -1;
}

int initramfs_mkdir(struct vnode *dir_node, struct vnode **target, char *component_name)
{
    printf("/initramfs is read-only!!\n");
    return -1;
}

/* file operations */
int initramfs_write(struct file *file, void *buf, size_t len)
{
    printf("/initramfs is read-only!!\n");
    return -1;
}

int initramfs_read(struct file *file, void *buf, size_t len)
{
    return tmpfs_read(file, buf, len);
}

int initramfs_open(struct vnode *file_node, struct file **target)
{
    return tmpfs_open(file_node, target);
}

int initramfs_close(struct file *file)
{
    return tmpfs_close(file);
}