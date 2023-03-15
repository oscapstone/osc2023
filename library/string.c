#include "string.h"
#include "uart.h"
#include "helper.h"

void string_init(string *s)
{
    s->length = 0;

    char temp[MAX_LENGTH];
    *s->string = temp;
    for (int i = 0; i < MAX_LENGTH; i++)
    {
        s->string = '\0';
    }
}

void string_append(string *s, char c)
{
    if (s->length >= MAX_LENGTH)
    {
        return;
    }

    // don't append new line
    if (c == '\n')
    {
        return;
    }

    s->string[s->length++] = c;
    s->string[s->length] = '\0';
}

void string_clear(string *s)
{
    int length = min(MAX_LENGTH, s->length);
    for (char i = 0; i < length; i++)
    {
        s->string[i] = '\0';
    }
    s->length = 0;
}

char *string_get_string(string s)
{
    return s.string;
}