#ifndef _DTB_H_
#define _DTB_H_

#define uint32_t unsigned int

// manipulate device tree with dtb file format
// linux kernel fdt.h
#define FDT_BEGIN_NODE 0x00000001
#define FDT_END_NODE 0x00000002
#define FDT_PROP 0x00000003
#define FDT_NOP 0x00000004//一般用於覆蓋樹中的屬性或者節點，以將其從書中刪除。
#define FDT_END 0x00000009//標識structure block區域的結束

//about function pointer:https://chenhh.gitbooks.io/parallel_processing/content/cython/function_pointer.html
typedef void (*dtb_callback)(uint32_t node_type, char *name, void *value, uint32_t name_size);
void traverse_device_tree(void *base, dtb_callback callback);  //traverse dtb tree

uint32_t uint32_endian_big2lttle(uint32_t data);

void dtb_callback_show_tree(uint32_t node_type, char *name, void *value, uint32_t name_size);
void dtb_callback_initramfs(uint32_t node_type, char *name, void *value, uint32_t name_size);

#endif
