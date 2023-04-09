int log2(int x) {
    int ret = 0;
    while (x >>= 1)
        ret++;
    return ret;
}

int pow2(int x) {
    return (1 << x);
}