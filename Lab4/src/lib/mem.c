void *memset(void *dest, register int val, int len)
{
    register unsigned char *ptr = (unsigned char *)dest;
    while (len-- > 0)
        *ptr++ = val;
    return dest;
}

int memcmp(void *s1, void *s2, int n)
{
    unsigned char *a = s1, *b = s2;
    while (n-- > 0)
    {
        if (*a != *b)
        {
            return *a - *b;
        }
        a++;
        b++;
    }
    return 0;
}