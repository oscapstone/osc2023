#include "fs/vfs.h"
#include "fs/dev_framebuffer.h"
#include "uart.h"
#include "malloc.h"
#include "mbox.h"
#include "string.h"

#define MBOX_REQUEST 0
#define MBOX_CH_PROP 8
#define MBOX_TAG_LAST 0

//The following code is for mailbox initialize used in lab7.
unsigned int width, height, pitch, isrgb; /* dimensions and channel order */
unsigned char *lfb;                       /* raw frame buffer address */

struct file_operations dev_framebuffer_operations = {dev_framebuffer_write, (void *)op_deny, dev_framebuffer_open, dev_framebuffer_close, vfs_lseek64, (void *)op_deny};

int init_dev_framebuffer()
{
    //The following code is for mailbox initialize used in lab7.
    mbox[0] = 35 * 4;
    mbox[1] = MBOX_REQUEST;

    mbox[2] = 0x48003; // set phy wh
    mbox[3] = 8;
    mbox[4] = 8;
    mbox[5] = 1024; // FrameBufferInfo.width
    mbox[6] = 768;  // FrameBufferInfo.height

    mbox[7] = 0x48004; // set virt wh
    mbox[8] = 8;
    mbox[9] = 8;
    mbox[10] = 1024; // FrameBufferInfo.virtual_width
    mbox[11] = 768;  // FrameBufferInfo.virtual_height

    mbox[12] = 0x48009; // set virt offset
    mbox[13] = 8;
    mbox[14] = 8;
    mbox[15] = 0; // FrameBufferInfo.x_offset
    mbox[16] = 0; // FrameBufferInfo.y.offset

    mbox[17] = 0x48005; // set depth
    mbox[18] = 4;
    mbox[19] = 4;
    mbox[20] = 32; // FrameBufferInfo.depth

    mbox[21] = 0x48006; // set pixel order
    mbox[22] = 4;
    mbox[23] = 4;
    mbox[24] = 1; // RGB, not BGR preferably

    mbox[25] = 0x40001; // get framebuffer, gets alignment on request
    mbox[26] = 8;
    mbox[27] = 8;
    mbox[28] = 4096; // FrameBufferInfo.pointer
    mbox[29] = 0;    // FrameBufferInfo.size

    mbox[30] = 0x40008; // get pitch
    mbox[31] = 4;
    mbox[32] = 4;
    mbox[33] = 0; // FrameBufferInfo.pitch

    mbox[34] = MBOX_TAG_LAST;

    // this might not return exactly what we asked for, could be
    // the closest supported resolution instead
    if (mbox_call(MBOX_CH_PROP) && mbox[20] == 32 && mbox[28] != 0)
    {
        mbox[28] &= 0x3FFFFFFF; // convert GPU address to ARM address
        width = mbox[5];        // get actual physical width
        height = mbox[6];       // get actual physical height
        pitch = mbox[33];       // get number of bytes per line
        isrgb = mbox[24];       // get the actual channel order
        lfb = PHYS_TO_VIRT((void *)((unsigned long)mbox[28]));
    }
    else
    {
        uart_puts("Unable to set screen resolution to 1024x768x32\n");
    }

    return register_dev(&dev_framebuffer_operations);
}

int dev_framebuffer_write(struct file *file, const void *buf, size_t len)
{
    lock();
    if (len + file->f_pos > pitch * height)
    {
        uart_printf("????\r\n");
        len = pitch * height - file->f_pos;
    }
    memcpy(lfb + file->f_pos, buf, len);
    file->f_pos += len;
    unlock();
    return len;
}

int dev_framebuffer_open(struct vnode *file_node, struct file **target)
{
    (*target)->f_pos = 0;
    (*target)->vnode = file_node;
    (*target)->f_ops = &dev_framebuffer_operations;
    return 0;
}

int dev_framebuffer_close(struct file *file)
{
    kfree(file);
    return 0;
}