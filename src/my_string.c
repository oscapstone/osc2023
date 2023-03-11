



/**
 * Helper function to convert ASCII octal number into binary
 * s string
 * n number of digits
 */
int oct2bin(char *s, int n)
{
    int r=0;
    while(n-- > 0) {
        r <<= 3;
        r += *s++ - '0';
    }
    return r;
}

int strcmp(const char *a, const char *b)
{
    while (*a) {
        if (*a != *b)
            break;
        a++;
        b++;
    }
    return *(const unsigned char *)a - *(const unsigned char *)b;
}