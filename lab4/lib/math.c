int pow(int base, int pow)
{
    int ret = 1;
    for (int i = 0; i < pow; i++)
    {
        ret *= base;
    }
    return ret;
}