#ifndef UTILS_H
#define UTILS_H

#define STR(x) #x
#define XSTR(x) STR(x)

unsigned int endian_big2little(unsigned int value);
unsigned int parse_hex_str(char *s, unsigned int max_len);

#endif