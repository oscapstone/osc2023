char* dtb_base;

void load() {
    char *kernel = (char *)(0x80000);
    int kn_ptr = 0;

    int size = 0;
    int base = '0';
    char c;
    while (1) {
        c = uart_getc();
        if (c == '\n') {
            break;
        }
        size = size * 10 + c - base;
    }

    while (size--)
        kernel[kn_ptr++] = uart_getc();

    void (*run)(char *) = (void *)kernel;
    run(dtb_base);
}