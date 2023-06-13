#include "dtb.h"
#include "utils.h"
#include "stdint.h"
#include "initramfs.h"
#include "uart.h"
#include "mmu.h"
struct fdt _fdt;
uint32_t fdt2hou(unsigned char *s) 
{
    uint32_t res = s[3];
    res |= s[2] << 8;
    res |= s[1] << 16;
    res |= s[0] << 24;
    return res;
}

/*on success, return 0*/
int fdt_init(struct fdt *fdt, void *head_addr) 
{
    uart_write_string("in fdt init\n");
    fdt->head_addr = head_addr;
    dtb_header_t *head = (dtb_header_t *)head_addr;
    if (fdt2hou(&(head->magic)) != 0xd00dfeed) {
        uart_write_string("fdt magic number != 0xd00dfeed\n");
        return 1;
    }
    fdt->fdt_struct_start = (char *)head_addr + fdt2hou(&(head->off_dt_struct));
    fdt->strings_start = (char *)head_addr + fdt2hou(&(head->off_dt_strings));
    fdt->total_size = fdt2hou(&(head->totalsize));
    fdt->fdt_traverse = _fdt_traversal;
    fdt->fdt_print = _fdt_print;
    fdt->end_addr = (char *)head_addr + fdt2hou(&(head->totalsize));
    return 0;
}

void initramfs_fdt_cb(struct fdt *self, dtb_node_t *node, dtb_property_t *prop, void *data) 
{
    if (prop) {
        char *prop_name = (char *)self->strings_start + fdt2hou(&(prop->nameoff));
        // uart_write_string(prop_name);
        //https://www.kernel.org/doc/Documentation/devicetree/bindings/chosen.txt
        if (strcmp(prop_name, "linux,initrd-start") == 0) {
            uart_write_string(prop_name);
            uart_write_string("\n");
            
            cpio_addr = (char *)PA2VA(fdt2hou(&(prop->data)));
        } else if (strcmp(prop_name, "linux,initrd-end") == 0) {
            uart_write_string(prop_name);
            uart_write_string("\n");
            
            cpio_end = (char *)PA2VA(fdt2hou(&(prop->data)));
        }
    }
}

int _fdt_traversal(struct fdt *self, fdt_callback_t cb, void *data)
{
    dtb_header_t *head = (dtb_header_t *)(self->head_addr);
    char *p = (char *)(self->fdt_struct_start);
    uint32_t *end = (uint32_t *)(self->head_addr) + (self->total_size);
    uint32_t tag;
    int level = 0;
    dtb_node_t *node_header = NULL;
    dtb_property_t *prop = NULL;
    while (p < end) {
        p = ALIGN((uint64_t)p, 4);
        
        tag = fdt2hou((char *)p);
        switch (tag)
        {
        case FDT_BEGIN_NODE:
            /*
            If the node is a FDT_BEGIN_NODE type, 
            the next integer in the sequence is 
            the offset to the node's name in the 
            string block. The name of the node is 
            a null-terminated string that identifies
             the hardware component the node represents.
            */
            node_header = p;
            cb(self, node_header, NULL, data);
            level++;
            p += ALIGN(sizeof(dtb_node_t) + (uint32_t)strlen(node_header->name)+1, 4);
            // p += sizeof(dtb_node_t) + (uint32_t)strlen(node_header->name)+1;
            // while (*p != '\0') p++;
            break;
        case FDT_END_NODE:
            /*
            The sequence of nodes and properties continues until the FDT_END_NODE type is encountered, at which point the sequence ends and processing returns to the parent node.
            */
            level--;
            node_header = NULL;
            prop = NULL;
            p += sizeof(dtb_node_t);
            break;
        /*
        After the name, the node can contain any number of properties, each of which is represented by a sequence of integers. The first integer is the property's type, which can be either FDT_PROP or FDT_NOP
        */
        case FDT_PROP:
            /*
            If the property is a FDT_PROP type, the next integer is the offset to the property's name in the string block. The name of the property is a null-terminated string that describes the characteristic of the hardware component.
            After the name, the property contains the binary data that describes the characteristic of the hardware component. The format of the binary data is defined by the Device Tree Specification.
            */
            prop = p;
            // uart_write_string(prop->data);
            // uart_write_string("\n");
            cb(self, node_header, prop, data);
            p += sizeof(dtb_property_t);
            p += fdt2hou(&(prop->len));
            break;
        case FDT_NOP:
            /*
            If the property is a FDT_NOP type, it is an alignment property that pads the sequence to a 4-byte boundary.
            */
            p += sizeof(dtb_node_t);
            break;
        case FDT_END:
            /*
            The flattened device tree ends with an FDT_END type, which marks the end of the sequence.
            */
            return 0;
            break;
        default:
            return 1;
            break;
        }
    }
    return 0;
}

static void print_spaces(int n)
{
    while (n--) {
        kuart_write(' ');
    }
}

int _fdt_print(struct fdt *self)
{
    dtb_header_t *head = (dtb_header_t *)(self->head_addr);
    char *p = (char *)(self->fdt_struct_start);
    uint32_t *end = (uint32_t *)(self->head_addr) + (self->total_size);
    uint32_t tag;
    int level = 0;
    dtb_node_t *node_header = NULL;
    dtb_property_t *prop = NULL;
    while (p < end) {
        p = ALIGN((uint64_t)p, 4);
        
        tag = fdt2hou((char *)p);
        switch (tag)
        {
        case FDT_BEGIN_NODE:
            /*
            If the node is a FDT_BEGIN_NODE type, 
            the next integer in the sequence is 
            the offset to the node's name in the 
            string block. The name of the node is 
            a null-terminated string that identifies
             the hardware component the node represents.
            */
            print_spaces(level * 2);
            node_header = p;
            uart_write_string("node: ");
            uart_write_string(node_header->name);
            uart_write_string("\n");
            level++;
            p += ALIGN(sizeof(dtb_node_t) + (uint32_t)strlen(node_header->name)+1, 4);
            // p += sizeof(dtb_node_t) + (uint32_t)strlen(node_header->name)+1;
            // while (*p != '\0') p++;
            break;
        case FDT_END_NODE:
            /*
            The sequence of nodes and properties continues until the FDT_END_NODE type is encountered, at which point the sequence ends and processing returns to the parent node.
            */
            level--;
            node_header = NULL;
            prop = NULL;
            p += sizeof(dtb_node_t);
            break;
        /*
        After the name, the node can contain any number of properties, each of which is represented by a sequence of integers. The first integer is the property's type, which can be either FDT_PROP or FDT_NOP
        */
        case FDT_PROP:
            /*
            If the property is a FDT_PROP type, the next integer is the offset to the property's name in the string block. The name of the property is a null-terminated string that describes the characteristic of the hardware component.
            After the name, the property contains the binary data that describes the characteristic of the hardware component. The format of the binary data is defined by the Device Tree Specification.
            */
            print_spaces(level * 2 + 2);
            prop = p;
            // uart_write_string(prop->data);
            // uart_write_string("\n");
            char *prop_name = (char *)self->strings_start + fdt2hou(&(prop->nameoff));
            uart_write_string("prop: ");
            uart_write_string(prop_name);
            uart_write_string("\n");
            p += sizeof(dtb_property_t);
            p += fdt2hou(&(prop->len));
            break;
        case FDT_NOP:
            /*
            If the property is a FDT_NOP type, it is an alignment property that pads the sequence to a 4-byte boundary.
            */
            p += sizeof(dtb_node_t);
            break;
        case FDT_END:
            /*
            The flattened device tree ends with an FDT_END type, which marks the end of the sequence.
            */
            return 0;
            break;
        default:
            return 1;
            break;
        }
    }
    return 0;
}


