#ifndef _BUILTIN_H
#define _BUILTIN_H

void _help(int mode);
void _hello(void);
void _hwinfo(void);
void _reboot(void);
void _echo(char *shell_buf);
void _ls(uint64 _initramfs_addr);
void _cat(uint64 _initramfs_addr, char *filename);
void _parsedtb(char *fdt_base);
void *_malloc(char *size);
void _exec(uint64 _initramfs_addr, char *filename);
void _chmod_uart();
int _setTimeout(char *shell_buf);

#endif