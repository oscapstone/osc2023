#include <stddef.h>

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

int strlen(const char *str)
{
    const unsigned char *s = (const unsigned char *)str;
    unsigned int len = 0;

    while (1)
    {
        if (*s++ == '\0')
            return len;
        len++;
    }
}

char *strcpy(char *destination, const char *source)
{
    // return if no memory is allocated to the destination
    if (destination == NULL)
    {
        return NULL;
    }

    // take a pointer pointing to the beginning of the destination string
    char *ptr = destination;

    // copy the C-string pointed by source into the array
    // pointed by destination
    while (*source != '\0')
    {
        *destination = *source;
        destination++;
        source++;
    }

    // include the terminating null character
    *destination = '\0';

    // the destination is returned by standard `strcpy()`
    return ptr;
}

// A simple atoi() function
int atoi(char *str)
{
    // Initialize result
    int res = 0;

    // Iterate through all characters
    // of input string and update result
    // take ASCII character of corresponding digit and
    // subtract the code from '0' to get numerical
    // value and multiply res by 10 to shuffle
    // digits left to update running total
    for (int i = 0; str[i] != '\0'; ++i)
        res = res * 10 + str[i] - '0';

    // return result.
    return res;
}
