
int strcmp(char *a, char *b)
{
    while (*a)
    {
        if (*a != *b)
        {
            break;
        }
        a++;
        b++;
    }
    return *a - *b;
}
