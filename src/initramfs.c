#include "initramfs.h"
#include "uart.h"
#include "utils.h"
#include "thread.h"
#include "mm.h"
#include "syscall.h"
char *cpio_addr, *cpio_end;
struct initramfs _initramfs;
extern void run_user_program(void *prog_addr, void *stack_addr);

void _cpio_ls(struct initramfs *self, char *path) 
{
    char *ptr = self->addr;

    if (strncmp(ptr, "070701", 6) != 0) {
        uart_write_string("Not a valid New ASCII Format Cpio archive file\n");
        return;
    }
    // ptr += 6;
    unsigned file_size, path_size, header_path_size;
    char *file_path;
    struct cpio_newc_header *header;
    while (1) {
        header = ptr;
        
        file_size = hex2unsigned(&(header->c_filesize));
        file_path = ptr + sizeof(struct cpio_newc_header);

        if (strncmp(file_path, "TRAILER!!!", 10) == 0) {
            //end of archive
            break;
        }

        path_size = hex2unsigned(&(header->c_namesize));
        header_path_size = sizeof(struct cpio_newc_header) + path_size;
        if (strstartswith(file_path, path)) {
            uart_write_string(file_path);
            uart_write_string("\n");
        }
        // skip the file content
        /*
         This expression adds 3 to the size of an integer to round 
         it up to the nearest multiple of 4 bytes,
         then applies the mask ~3 to set the last two bits to 0.
        */
        ptr += ((file_size + 3) & ~3) + ((header_path_size + 3) & ~3);
    }
}

struct cpio_newc_header *_cpio_find_file(struct initramfs *self, char *path)
{
    char *ptr = self->addr;
    if (strncmp(ptr, "070701", 6) != 0) {
        uart_write_string("Not a valid New ASCII Format Cpio archive file\n");
        return;
    }
    unsigned file_size, path_size, header_path_size;
    char *file_path;
    struct cpio_newc_header *header;
    while (1) {
        header = ptr;
        
        file_path = ptr + sizeof(struct cpio_newc_header);
        uart_write_string(file_path);
        uart_write_string("\n");
        if (strcmp(file_path, path) == 0) {
            return header;
        }

        if (strncmp(file_path, "TRAILER!!!", 10) == 0) {
            //end of archive
            break;
        }

        file_size = hex2unsigned(&(header->c_filesize));
        path_size = hex2unsigned(&(header->c_namesize));
        header_path_size = sizeof(struct cpio_newc_header) + path_size;
        // skip the file content
        /*
         This expression adds 3 to the size of an integer to round 
         it up to the nearest multiple of 4 bytes,
         then applies the mask ~3 to set the last two bits to 0.
        */
        ptr += ((file_size + 3) & ~3) + ((header_path_size + 3) & ~3);
    }
    return NULL;
}

char *_cpio_file_content(struct initramfs *self, char *path, size_t *fsize)
{
    char *ptr = self->addr;

    if (strncmp(ptr, "070701", 6) != 0) {
        uart_write_string("Not a valid New ASCII Format Cpio archive file\n");
        return NULL;
    }

    struct cpio_newc_header *header = _cpio_find_file(self, path);
    if (header == NULL) {
        uart_write_string("File Not Found!\n");
        return NULL;
    }
    size_t file_size, path_size, header_path_size;
    file_size = hex2unsigned(&(header->c_filesize));
    path_size = hex2unsigned(&(header->c_namesize));
    header_path_size = sizeof(struct cpio_newc_header) + path_size;
    header_path_size = ALIGN(header_path_size, 4);//((header_path_size + 3) & ~3);
    char *content_ptr = (char *)header + header_path_size;
    *fsize = file_size;
    return content_ptr;
}

void _cpio_cat(struct initramfs *self, char *path) 
{
    size_t file_size;
    char *content_ptr = self->file_content(self, path, &file_size);
    for (int i = 0; i < file_size; i++, content_ptr++) {
        kuart_write(*content_ptr);
    }
    // uart_write_string("\n");
}

int _cpio_exec(struct initramfs *self, char *argv[])
{
    char *ptr = self->addr;

    char *path = argv[0];

    unsigned file_size, path_size, header_path_size;

    // char *content_ptr = self->file_content(self, path, &file_size);
    ///////////////////////////////////////////////////

    if (strncmp(ptr, "070701", 6) != 0) {
        uart_write_string("Not a valid New ASCII Format Cpio archive file\n");
        return -1;
    }

    struct cpio_newc_header *header = _cpio_find_file(self, path);
    if (header == NULL) {
        uart_write_string("File Not Found!\n");
        return -1;
    }
    file_size = hex2unsigned(&(header->c_filesize));
    path_size = hex2unsigned(&(header->c_namesize));
    header_path_size = sizeof(struct cpio_newc_header) + path_size;
    header_path_size = ALIGN(header_path_size, 4);
    char *content_ptr = (char *)header + header_path_size;
    /////////////////////////////////////////////////////
    // char *load_addr = load_program(content_ptr, file_size);
    /////////////////////////////////////////////////////
    task_t *current = get_current_thread();
    if (current->user_text != NULL) {
        free_page(current->user_text);
        current->user_text = NULL;
    }
    size_t page_needed = ALIGN(file_size, PAGE_SIZE) / PAGE_SIZE;
    char *load_addr = alloc_pages(page_needed);
    if (load_addr == NULL) {
        uart_write_string("No enough space for loading program.\n");
        return NULL;
    }
    memcpy(load_addr, content_ptr, file_size);
    current->user_text = load_addr;
    mappages(current->pgd, 0x0, file_size, VA2PA(load_addr));

    //////////////////////////////////////////////////////
    // run_user_prog(load_addr);
    //////////////////////////////////////////////////////
    mappages(current->pgd, USER_STK_LOW, STACKSIZE, 0);
    update_pgd(VA2PA(current->pgd));
    //write lr
    __asm__ __volatile__("mov x30, %[value]"
                         :
                         : [value] "r" (exit)
                         : "x30");
    //write fp
    __asm__ __volatile__("mov x29, %[value]"
                         :
                         : [value] "r" (USER_STK_HIGH)
                         : "x29");
    //allow interrupt
    write_sysreg(spsr_el1, 0);
    write_sysreg(elr_el1, 0x0);
    write_sysreg(sp_el0, USER_STK_HIGH);
    asm volatile("eret");
    return -1;
}

void init_initramfs(struct initramfs *fs) 
{
    fs->addr = cpio_addr;
    fs->ls = _cpio_ls;
    fs->cat = _cpio_cat;
    fs->file_content = _cpio_file_content;
    fs->exec = _cpio_exec;
}
