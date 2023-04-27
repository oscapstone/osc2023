#include "string.h"

int stringcmp(const char *p1, const char *p2)
{
    const unsigned char *s1 = (const unsigned char *) p1;
    const unsigned char *s2 = (const unsigned char *) p2;
    unsigned char c1, c2;
    
    do {
        c1 = (unsigned char) *s1++;
        c2 = (unsigned char) *s2++;
        if (c1 == '\0')
        return c1 - c2;
    } while (c1 == c2);
    
    return c1 - c2;
}

int stringncmp(const char *p1, const char *p2, unsigned int n)
{
    for (int i=0; i<n; i++) {
        if (p1[i] != p2[i]) return p1[i] - p2[i];
        if (p1[i] == 0) return 0;
    }
    return 0;
}

unsigned int strlen(const char *s) {
    
    unsigned int l = 0;
    
    while (*s != '\0') {
        l++;
        s++;
    }

    return l;

}