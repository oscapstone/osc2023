int min(int n1, int n2)
{
    return n1 < n2 ? n1 : n2;
}

int string_length(char *s)
{
    int length = 0;

    for (; *s != '\0'; *s++, length++)
        ;

    return length;
}

char string_compare(char *s1, char *s2)
{
    int len1 = string_length(s1);
    int len2 = string_length(s2);

    if (len1 != len2)
    {
        return 0;
    }

    for (int i = 0; i < len1; i++)
    {
        if (s1[i] != s2[i])
        {
            return 0;
        }
    }

    return 1;
}

void set(long addr, unsigned int value)
{
    volatile unsigned int *point = (unsigned int *)addr;
    *point = value;
}

void nop()
{
    asm volatile("nop");
}