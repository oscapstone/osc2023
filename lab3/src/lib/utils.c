#include <utils.h>

void wait_cycles(uint32_t times){
    while(times--) asm volatile("nop");
}

uint32_t hex2u32_8(char *buf){
    uint32_t num = 0;
    for(int i = 0; i < 8; i++){
        num <<= 4;
        num += (buf[i] >= 'A' ? buf[i] - 'A' + 10 : buf[i] - '0');
    }
    return num;
}

uint16_t ntohs(uint16_t tmp){
    return (tmp << 8) | (tmp >> 8);
}

uint32_t ntohl(uint32_t tmp){
    return ((uint32_t)(ntohs(tmp & 0xffffLL)) << 16) | (ntohs((tmp >> 16) & 0xffffLL));
}

uint64_t ntohll(uint64_t tmp){
    return ((uint64_t)(ntohl(tmp & 0xffffffffLL)) << 32) | (ntohl((tmp >> 32) & 0xffffffffLL));
}

uint16_t htons(uint16_t tmp){
    return  (tmp >> 8) | (tmp << 8);
}

uint32_t htonl(uint32_t tmp){
    return (ntohs((tmp >> 16) & 0xffffLL)) | ((uint32_t)(ntohs(tmp & 0xffffLL)) << 16);
}

uint64_t htonll(uint64_t tmp){
    return (ntohl((tmp >> 32) & 0xffffffffLL)) | ((uint64_t)(ntohl(tmp & 0xffffffffLL)) << 32);
}