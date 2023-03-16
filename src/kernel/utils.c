#include "type.h"
#include "utils.h"
void memcpy(void* dest, void *src, unsigned long long size) {
    for(int i = 0; i < size; i ++) {
        *(char*)(dest + i) = *(char*)(src + i);
    }
}
void memset(void *dest, char val, unsigned int size) {
    for(int i = 0; i < size; i ++) {
        *(char*)(dest + i) = val;
    }
}
int strncmp(const char *s1,const char *s2, unsigned int maxlen) {
    int i;
    for(i = 0; i < maxlen && s1[i] != '\0' && s2[i] != '\0'; i ++) {
        if(s1[i] < s2[i]) {
            return -1;
        }
        if(s1[i] > s2[i]) {
            return 1;
        }
    }
    if(s1[i] == '\0' && s2[i] != '\0') {
        return -1;
    }
    if(s1[i] != '\0' && s2[i] == '\0') {
        return 1;
    }
    return 0;
}

int strlen(const void *buf) {
    int ret = 0;
    for(const char *ch = (const char *)buf; *ch != '\0'; ch ++) {
        ret += 1;
    }
    return ret;
}

uint64_t ntohl(uint64_t t) {
    return ((uint64_t)(ntohi(t & 0xffffffffLL)) << 32) | (ntohi((t >> 32) & 0xffffffffLL));
}
uint32_t ntohi(uint32_t t) {
    return ((uint32_t)(ntohs(t & 0xffffLL)) << 16) | (ntohs((t >> 16) & 0xffffLL));
}
uint16_t ntohs(uint16_t t) {
    return (t << 8) | (t >> 8);
}