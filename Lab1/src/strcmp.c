int strcmp(const char *str1, const char *str2)
{
    const unsigned char *s1 = (const unsigned char *)str1;
    const unsigned char *s2 = (const unsigned char *)str2;
    unsigned char c1, c2;

    while (1)
    {
        c1 = *s1++;
        c2 = *s2++;
        if (c1 != c2)
            break;
        if (c1 == '\0')
            return 0;
    }

    return c1 - c2;
}