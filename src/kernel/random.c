#include "random.h"
#include "type.h"

static uint32_t seed;
static uint32_t MT[624];
static uint8_t first;
static int index = 0;

void srand(uint32_t sd) {
    int i = 0;
    MT[0] = sd;
    for(int i = 1; i < 624; i ++) {
        MT[i] = 1812433253 *(MT[i-1] ^ ((MT[i-1] >> 30))) + i;
    }
}
void generate_number() {
    for (int i = 0; i < 624; i ++) {
        int y = (MT[i] & 0x80000000) + (MT[(i+1) % 624] & 0x7fffffff);
        MT[i] = MT[(i + 397) % 624] ^ ((y >> 1));
        if (y & 1) { // y is odd
            MT[i] = MT[i] ^ (2567483615);
        }
    }
}

uint32_t rand() {
    // very weird random
    if(index == 0) {
        generate_number();
    }
    int y = MT[index];
    y = y ^ ((y >> 11));
    y = y ^ ((y << 7) & (2636928640));
    y = y ^ ((y << 15) & (4022730752));
    y = y ^ (y >> 18);

    index = (index + 1) % 624;
    return y;
}