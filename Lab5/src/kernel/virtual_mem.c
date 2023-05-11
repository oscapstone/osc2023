extern void tcr_init();
extern void mair_init();
extern void identity_init();

void virtual_mem_init()
{
    tcr_init();
    mair_init();
    identity_init();
}