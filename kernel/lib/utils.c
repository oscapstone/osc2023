#include "utils.h"

unsigned int endian_big2little(unsigned int value)
{
    char *r = (char *)&value;
    return (r[3] << 0) | (r[2] << 8) | (r[1] << 16) | (r[0] << 24);
}

/* Parse an ASCII hex string into an integer. (big endian)*/
unsigned int parse_hex_str(char *s, unsigned int max_len)
{
    unsigned int r = 0;

    for (unsigned int i = 0; i < max_len; i++)
    {
        r *= 16;
        if (s[i] >= '0' && s[i] <= '9')
            r += s[i] - '0';
        else if (s[i] >= 'a' && s[i] <= 'f')
            r += s[i] - 'a' + 10;
        else if (s[i] >= 'A' && s[i] <= 'F')
            r += s[i] - 'A' + 10;
        else
            return r;
    }
    return r;
}