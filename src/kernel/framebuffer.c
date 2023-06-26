#include "mbox/mbox.h"
#include "fs/vfs.h"
#include "mem/mem.h"
#include "framebuffer.h"
#include "mmu/mmu-def.h"

#define MBOX_REQUEST 0
#define MBOX_CH_PROP 8
#define MBOX_TAG_LAST 0

unsigned int __attribute__((aligned(16))) framebuffer_mbox[36];
unsigned int width, height, pitch, isrgb; /* dimensions and channel order */
unsigned char *lfb;                       /* raw frame buffer address */

struct filesystem framebuffer_fs = {
    .name = "framebuffer",
    .init_fs = &init_framebuffer,
    .root = NULL,
    .setup_mount = &framebuffer_setup_mount
};


struct file_operations framebuffer_fops = {
    .close = &framebuffer_close,
    .lseek64 = &framebuffer_lseek64,
    .open = &framebuffer_open,
    .read = &vops_not_support,
    .stat = &vops_not_support,
    .write = &framebuffer_write,
    .ioctl = &framebuffer_ioctl
};

struct vnode_operations framebuffer_vops = {
    .create = &fops_not_support,
    .lookup = &fops_not_support,
    .mkdir = &fops_not_support
};

int init_framebuffer(struct filesystem* fs) {
    fs->root = (struct vnode*)kmalloc(sizeof(struct vnode));
    ds_list_head_init(&(fs->root->ch_list));
    ds_list_head_init(&(fs->root->v_head));
    fs->root->flag = VFS_FILE;
    fs->root->internal = NULL;
    fs->root->v_ops = &framebuffer_vops;
    fs->root->f_ops = &framebuffer_fops;
    fs->root->mount = NULL;

    framebuffer_mbox[0] = 35 * 4;
    framebuffer_mbox[1] = MBOX_REQUEST;

    framebuffer_mbox[2] = 0x48003; // set phy wh
    framebuffer_mbox[3] = 8;
    framebuffer_mbox[4] = 8;
    framebuffer_mbox[5] = 1024; // FrameBufferInfo.width
    framebuffer_mbox[6] = 768;  // FrameBufferInfo.height

    framebuffer_mbox[7] = 0x48004; // set virt wh
    framebuffer_mbox[8] = 8;
    framebuffer_mbox[9] = 8;
    framebuffer_mbox[10] = 1024; // FrameBufferInfo.virtual_width
    framebuffer_mbox[11] = 768;  // FrameBufferInfo.virtual_height

    framebuffer_mbox[12] = 0x48009; // set virt offset
    framebuffer_mbox[13] = 8;
    framebuffer_mbox[14] = 8;
    framebuffer_mbox[15] = 0; // FrameBufferInfo.x_offset
    framebuffer_mbox[16] = 0; // FrameBufferInfo.y.offset

    framebuffer_mbox[17] = 0x48005; // set depth
    framebuffer_mbox[18] = 4;
    framebuffer_mbox[19] = 4;
    framebuffer_mbox[20] = 32; // FrameBufferInfo.depth

    framebuffer_mbox[21] = 0x48006; // set pixel order
    framebuffer_mbox[22] = 4;
    framebuffer_mbox[23] = 4;
    framebuffer_mbox[24] = 1; // RGB, not BGR preferably

    framebuffer_mbox[25] = 0x40001; // get framebuffer, gets alignment on request
    framebuffer_mbox[26] = 8;
    framebuffer_mbox[27] = 8;
    framebuffer_mbox[28] = 4096; // FrameBufferInfo.pointer
    framebuffer_mbox[29] = 0;    // FrameBufferInfo.size

    framebuffer_mbox[30] = 0x40008; // get pitch
    framebuffer_mbox[31] = 4;
    framebuffer_mbox[32] = 4;
    framebuffer_mbox[33] = 0; // FrameBufferInfo.pitch

    framebuffer_mbox[34] = MBOX_TAG_LAST;

    // this might not return exactly what we asked for, could be
    // the closest supported resolution instead
    if (mbox_call(MBOX_CH_PROP, framebuffer_mbox) && framebuffer_mbox[20] == 32 && framebuffer_mbox[28] != 0) {
        framebuffer_mbox[28] &= 0x3FFFFFFF; // convert GPU address to ARM address
        width = framebuffer_mbox[5];        // get actual physical width
        height = framebuffer_mbox[6];       // get actual physical height
        pitch = framebuffer_mbox[33];       // get number of bytes per line
        isrgb = framebuffer_mbox[24];       // get the actual channel order
        lfb = (void *)((unsigned long)framebuffer_mbox[28]);
        lfb = kernel_pa2va(lfb);
    } else {
        uart_send_string("Unable to set screen resolution to 1024x768x32\n");
    }
}

int framebuffer_ioctl(struct file *f, unsigned long request, void *buf) {
    return 0;
}

int framebuffer_close(struct file *f) {
    kfree(f);
    return 0;
}
int framebuffer_lseek64(struct file *f, int offset, int whence) {
    if(whence == SEEK_SET) {
        f->f_pos = offset;
        return 0;
    } else {
        return 0;
    }
}

int framebuffer_open(struct vnode *file_node, struct file **target) {
    struct file *ret = (struct file *)kmalloc(sizeof(struct file));
    ret->f_ops = &framebuffer_fops;
    ret->f_pos = 0;
    ret->flags = 0777;
    ret->vnode = framebuffer_fs.root;
    *target = ret;
    return 0;
}


int framebuffer_write(struct file *f, void *buf, long len) {
    for(int i = 0; i < len; i ++, f->f_pos ++) {
       *(char*)(lfb + f->f_pos) = *(char*)(buf + i);
    }
    return len;
}

int framebuffer_setup_mount(struct filesystem* fs, struct mount *mnt) {
    ds_list_head_init(&(fs->root->ch_list));
    ds_list_head_init(&(fs->root->v_head));
    fs->root->flag = VFS_FILE;
    fs->root->mount = mnt;
    mnt->root = fs->root;
    mnt->fs = fs;
}


void setup_dev_framebuffer() {
    register_filesystem(&framebuffer_fs);
    vfs_mkdir("/dev/framebuffer");
    vfs_mount("/dev/framebuffer", "framebuffer");
}
