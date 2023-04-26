int hex2int(char *s, int n)
{
    int r = 0;
    while (n-- > 0)
    {
        r <<= 4;
        if (*s >= '0' && *s <= '9')
            r += *s++ - '0';
        else if (*s >= 'A' && *s <= 'F')
            r += *s++ - 'A' + 10;
        else if (*s >= 'a' && *s <= 'f')
            r += *s++ - 'a' + 10;
    }
    return r;
}