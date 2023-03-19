#include "bcm2835/uart.h"
#include "init/fdt.h"
#include "init/salloc.h"
#include "string.h"

#define align4(x) (((unsigned int)(x) + 3U) & (~3U))
#define peek(x)   (get_big32u(x))

#define FDT_MAGIC       0xd00dfeedU

#define FDT_BEGIN_NODE  0x00000001U
#define FDT_END_NODE    0x00000002U
#define FDT_PROP        0x00000003U
#define FDT_NOP         0x00000004U
#define FDT_END         0x00000009U

static struct fdt * fdt = 0;

struct _fdt_header {
    char magic[4];
    char totalsize[4];
    char off_dt_struct[4];
    char off_dt_strings[4];
    char off_mem_rsvmap[4];
    char version[4];
    char last_comp_version[4];
    char boot_cpuid_phys[4];
    char size_dt_strings[4];
    char size_dt_struct[4];
} __attribute__((packed));

static inline void exchg_char(char * c, char * d) {
    *c ^= *d;
    *d ^= *c;
    *c ^= *d;
}

static inline unsigned int big2little32u(unsigned int n) {
    char * c = (char *)&n;
    exchg_char(c + 0, c + 3);
    exchg_char(c + 1, c + 2);
    return n;
}

static inline unsigned int get_big32u(char * addr) {
    unsigned int big = *(unsigned int *)addr;
    unsigned int little = big2little32u(big);
    return little;
}

static inline struct fdt_node * new_node() {
    struct fdt_node * node = (struct fdt_node *)simple_malloc(sizeof(struct fdt_node));
    node->name = "";
    node->level = 0;
    node->chdrn = 0;
    node->slbng = 0;
    node->prpty = 0;
    return node;
}

static inline char * skip_nop(char * addr) {
    while (get_big32u(addr) == FDT_NOP) {
        addr += 4;
    }
    return addr;
}

static inline char * skip_str(char * addr) {
    unsigned int off = align4(strlen(addr) + 1);
    return addr + off;
}

static inline char * consume(char * addr, unsigned int token) {
    while (get_big32u(addr) != token) {
        addr += 4;
        uart_puts("[*] FDT PARSING ERROR: ARRD = ");
        uart_pu32h((unsigned int)(unsigned long long int)addr);
        uart_puts(", TOKEN != ");
        uart_pu32h(token);
        uart_send('\n');
    }
    return addr + 4;
}

static char * fdt_append_prop(struct fdt_node * node, char * addr) {
    struct fdt_prop * prop = (struct fdt_prop *)simple_malloc(sizeof(struct fdt_prop));
    prop->next = node->prpty;
    node->prpty = prop;
    prop->name = fdt->addr + get_big32u(addr + 4);
    prop->len = get_big32u(addr);
    prop->val = addr + 8;
    return addr + 8 + align4(prop->len);
}

static char * _fdt_init(struct fdt_node * node, char * addr);

static inline char * fdt_append_child(struct fdt_node * node, char * addr) {
    struct fdt_node * child = new_node();
    child->level = node->level + 1;
    child->slbng = node->chdrn;
    node->chdrn = child;
    return _fdt_init(child, addr);
}

static char * _fdt_init(struct fdt_node * node, char * addr) {
    node->name = addr;
    addr = skip_str(addr);
    addr = skip_nop(addr);
    while (peek(addr) == FDT_PROP) {
        addr = consume(addr, FDT_PROP);
        addr = fdt_append_prop(node, addr);
        addr = skip_nop(addr);
    }
    while (peek(addr) == FDT_BEGIN_NODE) {
        addr = consume(addr, FDT_BEGIN_NODE);
        addr = fdt_append_child(node, addr);
        addr = consume(addr, FDT_END_NODE);
        addr = skip_nop(addr);
    }
    return addr;
}

void fdt_init(char * addr) {
    struct _fdt_header * header = (struct _fdt_header *)addr;
    if (fdt || get_big32u(header->magic) != FDT_MAGIC) {
        uart_puts("[*] BAD FDT ADDRESS !!!");
        return;
    }
    fdt = (struct fdt *)simple_malloc(sizeof(struct fdt));
    fdt->addr = addr;
    fdt->dt_struct = addr + get_big32u(header->off_dt_struct);
    fdt->dt_string = addr + get_big32u(header->off_dt_strings);

    addr = skip_nop(fdt->dt_struct);
    addr = consume(addr, FDT_BEGIN_NODE);

    fdt->root = new_node();
    addr = _fdt_init(fdt->root, addr);
    fdt->root->name = "ROOT NODE";

    addr = consume(addr, FDT_END_NODE);
    addr = consume(addr, FDT_END);
}

static void _fdt_traverse(
        struct fdt_node * node,
        void (* callback)(struct fdt_node * node),
        unsigned int (* cmp)(struct fdt_node * node)) {
    if (!node) {
        return;
    }
    if (cmp(node)) {
        callback(node);
    }
    _fdt_traverse(node->chdrn, callback, cmp);
    _fdt_traverse(node->slbng, callback, cmp);
}

void fdt_traverse(void (* callback)(struct fdt_node * node),
                  unsigned int (* cmp)(struct fdt_node * node)) {
    if (!fdt) {
        return;
    }
    _fdt_traverse(fdt->root, callback, cmp);
}