char *counter = (char *)0x10000000;

void *simple_malloc(unsigned int size)
{
    char *dest = counter;
    counter += size;
    return dest;
}
