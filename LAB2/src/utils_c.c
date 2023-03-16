#include "utils_c.h"
#include "mini_uart.h"
#include <stddef.h>

/*
    string part
*/
int utils_str_compare(const char *a, const char *b)
{
    char aa, bb;
    do
    {
        aa = (char)*a++;
        bb = (char)*b++;
        if (aa == '\0' || bb == '\0')
        {
            return aa - bb;
        }
    } while (aa == bb);
    return aa - bb;
}
void utils_newline2end(char *str) // 長字串截斷
{
    while (*str != '\0')
    {
        if (*str == '\n')
        {
            *str = '\0';
            return;
        }
        ++str;
    }
}

void utils_int2str_dec(int num, char *str)
{
    // num=7114 digit=4
    int digit = -1, temp = num;
    while (temp > 0)
    {
        temp /= 10;
        digit++;
    }
    for (int i = digit; i >= 0; i--)
    {
        int t = 1;
        for (int j = 0; j < i; j++)
        {
            t *= 10;
        }
        *str = '0' + num / t;
        num = num % t;
        str++;
    }
    *str = '\0';
}

void utils_uint2str_dec(unsigned int num, char *str)
{
    // num=7114 digit=4
    unsigned int temp = num;
    int digit = -1;
    while (temp > 0)
    {
        temp /= 10;
        digit++;
    }
    for (int i = digit; i >= 0; i--)
    {
        int t = 1;
        for (int j = 0; j < i; j++)
        {
            t *= 10;
        }
        *str = '0' + num / t;
        num = num % t;
        str++;
    }
    *str = '\0';
}
void utils_uint2str_hex(unsigned int num, char *str)
{
    // num=7114 digit=4
    unsigned int temp = num;
    int digit = -1;
    *str = '0';
    *str++;
    *str = 'x';
    *str++;
    if (num == 0)
    {
        *str = '0';
        str++;
    }
    else
    {
        while (temp > 0)
        {
            temp /= 16;
            digit++;
        }
        for (int i = digit; i >= 0; i--)
        {
            int t = 1;
            for (int j = 0; j < i; j++)
            {
                t *= 16;
            }
            if (num / t >= 10)
            {
                *str = '0' + num / t + 39;
            }
            else
            {
                *str = '0' + num / t;
            }
            num = num % t;
            str++;
        }
    }
    *str = '\0';
}

unsigned int utils_str2uint_dec(const char *str)
{
    unsigned int value = 0u;

    while (*str)
    {
        value = value * 10u + (*str - '0');
        ++str;
    }
    return value;
}

size_t utils_strlen(const char *s)
{
    size_t i = 0;
    while (s[i])
        i++;
    return i + 1;
}

/*
    reboot part
*/

void set(long addr, unsigned int value)
{
    volatile unsigned int *point = (unsigned int *)addr;
    *point = value;
}

void reset(int tick)
{                                     // reboot after watchdog timer expire
    set(PM_RSTC, PM_PASSWORD | 0x20); // full reset
    set(PM_WDOG, PM_PASSWORD | tick); // number of watchdog tick
}

void cancel_reset()
{
    set(PM_RSTC, PM_PASSWORD | 0); // full reset
    set(PM_WDOG, PM_PASSWORD | 0); // number of watchdog tick
}

/*
    others
*/

void align(void *size, size_t s) // aligned to 4 byte
{
    /*
        The pathname is followed by NUL bytes so that the total size of the fixed header plus pathname is a multiple of 4.
        Likewise, the file data is padded to a multiple of 4 bytes.
    */
    unsigned long *x = (unsigned long *)size;
    if ((*x) & (s - 1)) // 功能與 %4 一樣
    {
        (*x) += s - ((*x) & (s - 1)); // 補足4 - %4
    }
}

uint32_t align_up(uint32_t size, int alignment)
{
    return (size + alignment - 1) & -alignment;
}
/***********************************************************************************************/
/**
 * Display a string
 */
void printf(char *fmt, ...)
{
    __builtin_va_list args;
    __builtin_va_start(args, fmt);
    // we don't have memory allocation yet, so we
    // simply place our string after our code
    char *s = (char *)&_ebss;
    // use sprintf to format our string
    vsprintf(s, fmt, args);
    // print out as usual
    while (*s)
    {
        /* convert newline to carrige return + newline */
        if (*s == '\n')
            uart_send('\r');
        uart_send(*s++);
    }
}
/**
 * minimal sprintf implementation
 */
unsigned int vsprintf(char *dst, char *fmt, __builtin_va_list args)
{
    long int arg;
    int len, sign, i;
    char *p, *orig = dst, tmpstr[19];

    // failsafes
    if (dst == (void *)0 || fmt == (void *)0)
    {
        return 0;
    }

    // main loop
    arg = 0;
    while (*fmt)
    {
        // argument access
        if (*fmt == '%')
        {
            fmt++;
            // literal %
            if (*fmt == '%')
            {
                goto put;
            }
            len = 0;
            // size modifier
            while (*fmt >= '0' && *fmt <= '9')
            {
                len *= 10;
                len += *fmt - '0';
                fmt++;
            }
            // skip long modifier
            if (*fmt == 'l')
            {
                fmt++;
            }
            // character
            if (*fmt == 'c')
            {
                arg = __builtin_va_arg(args, int);
                *dst++ = (char)arg;
                fmt++;
                continue;
            }
            else
                // decimal number
                if (*fmt == 'd')
                {
                    arg = __builtin_va_arg(args, int);
                    // check input
                    sign = 0;
                    if ((int)arg < 0)
                    {
                        arg *= -1;
                        sign++;
                    }
                    if (arg > 99999999999999999L)
                    {
                        arg = 99999999999999999L;
                    }
                    // convert to string
                    i = 18;
                    tmpstr[i] = 0;
                    do
                    {
                        tmpstr[--i] = '0' + (arg % 10);
                        arg /= 10;
                    } while (arg != 0 && i > 0);
                    if (sign)
                    {
                        tmpstr[--i] = '-';
                    }
                    // padding, only space
                    if (len > 0 && len < 18)
                    {
                        while (i > 18 - len)
                        {
                            tmpstr[--i] = ' ';
                        }
                    }
                    p = &tmpstr[i];
                    goto copystring;
                }
                else
                    // hex number
                    if (*fmt == 'x')
                    {
                        arg = __builtin_va_arg(args, long int);
                        // convert to string
                        i = 16;
                        tmpstr[i] = 0;
                        do
                        {
                            char n = arg & 0xf;
                            // 0-9 => '0'-'9', 10-15 => 'A'-'F'
                            tmpstr[--i] = n + (n > 9 ? 0x37 : 0x30);
                            arg >>= 4;
                        } while (arg != 0 && i > 0);
                        // padding, only leading zeros
                        if (len > 0 && len <= 16)
                        {
                            while (i > 16 - len)
                            {
                                tmpstr[--i] = '0';
                            }
                        }
                        p = &tmpstr[i];
                        goto copystring;
                    }
                    else
                        // string
                        if (*fmt == 's')
                        {
                            p = __builtin_va_arg(args, char *);
                        copystring:
                            if (p == (void *)0)
                            {
                                p = "(null)";
                            }
                            while (*p)
                            {
                                *dst++ = *p++;
                            }
                        }
        }
        else
        {
        put:
            *dst++ = *fmt;
        }
        fmt++;
    }
    *dst = 0;
    // number of bytes written
    return dst - orig;
}

/**
 * Variable length arguments
 */
unsigned int sprintf(char *dst, char *fmt, ...)
{
    __builtin_va_list args;
    __builtin_va_start(args, fmt);
    return vsprintf(dst, fmt, args);
}