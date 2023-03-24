#ifndef	STRING_UTILS_H
#define	STRING_UTILS_H

int strcmp(const char *s1, const char *s2);
int string_hex_to_int(char *str, int len);
unsigned int strlen(const char *str);
unsigned int big_bytes_to_uint(char* ptr, int len);
char* strncpy(char *dest, const char *src, int n);

#endif /* STRING_UTILS_H */