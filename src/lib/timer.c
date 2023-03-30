#include <timer.h>
#include <utils.h>
#include <mini_uart.h>
#include <syscall.h>
#include <list.h>
#include <irq.h>

#define CORE0_TIMER_IRQ_CTRL 0x40000040
#define CORE0_IRQ_SOURCE 0x40000060
#define TIMER_NUM 32
uint64 timer_boot_cnt;
static int timer_dump = 0;

timer_node t_nodes[TIMER_NUM];
timer_meta t_meta;

static void timer_enable(){
    // Enable core0 cntp timer
    put32(CORE0_TIMER_IRQ_CTRL,2);
}

static void timer_disable(){
    // Disable core0 cntp timer
    put32(CORE0_TIMER_IRQ_CTRL,0);
}

static void timer_set_boot_cnt(){
    timer_boot_cnt = read_sysreg(cntpct_el0);
}

static void tn_free(timer_node *tn){
    if(!tn)
        return;

    // get idx of the elem in the array
    uint32 idx = get_elem_idx(tn, t_nodes);

    // set the bit because the element is available
    t_meta.t_status |= (1<<idx);
}

static void update_remain_time(){
    if(list_empty(&t_meta.lh))
        return;
    uint32 cntp_tval_el0 = read_sysreg(cntp_tval_el0);

    // count the difference between software counter and hardware counter
    uint32 t_dur = t_meta.t_interval - cntp_tval_el0;
    t_meta.t_interval = cntp_tval_el0;

    timer_node *entry;
    list_for_each_entry(entry, &t_meta.lh, lh){
        entry->time_left -= t_dur;
    }
}

static void timer_set(){
    if(list_empty(&t_meta.lh))
        return;
    uint64 daif = read_sysreg(DAIF);
    disable_interrupt();
    timer_node *tn = list_first_entry(&t_meta.lh, timer_node, lh);

    // set timer and metadata
    t_meta.t_interval = tn->time_left;
    write_sysreg(cntp_tval_el0, t_meta.t_interval);

    write_sysreg(DAIF, daif);
}

static int tn_insert(timer_node *tn){
    uint64 daif = read_sysreg(DAIF);
    int first;
    timer_node *entry;
    disable_interrupt();
    update_remain_time();
    first = 1;
    list_for_each_entry(entry, &t_meta.lh, lh){
        if(tn->time_left < entry->time_left)
            break;
        first = 0;
    }
    // insert the t_node to previous of entry
    list_add_tail(&tn->lh,&entry->lh);

    write_sysreg(DAIF, daif);
    return first;
}

static timer_node *tn_alloc(){
    uint64 daif;
    uint32 idx;

    // store the interrupt bits
    daif = read_sysreg(DAIF);
    disable_interrupt();

    // get the lowest bit idx with value 1
    idx = __builtin_ffs(t_meta.t_status)-1;

    if(idx < 0){
        write_sysreg(DAIF, daif);
        return NULL;
    }

    // mask out the bit to 0 because that t_node is in use.
    t_meta.t_status &= ~(1<<idx);
    write_sysreg(DAIF, daif);
    return &t_nodes[idx];
}

void timer_init(){
    timer_set_boot_cnt();
    INIT_LIST_HEAD(&t_meta.lh);
    t_meta.t_interval = 0;
    t_meta.t_status = 0xffffffff;

    add_timer(boot_time_callback,NULL,2);
}

void boot_time_callback()
{
    // uint32 core0_irq_src = get32(CORE0_IRQ_SOURCE);

    // if (core0_irq_src & 0x02) {
        // Set next timer before calling any functions which may interrupt
    uint32 cntfrq_el0 = read_sysreg(cntfrq_el0);
    uint64 cntpct_el0 = read_sysreg(cntpct_el0);
    timer_dump ++;
    // just dump two times
    if(timer_dump<=2){
        uart_printf("[Boot time: %lld seconds...]\r\n", (cntpct_el0 - timer_boot_cnt) / cntfrq_el0);
        add_timer(boot_time_callback, NULL ,2);
    }
    // }
}

void add_timer(void (*callback)(void *), void *data, uint32 timeval){
    // (callback)(data);
    // uart_printf("\r\nTimeout: %d\r\n", timeval);
    timer_node *tn = tn_alloc();
    uint32 cntfrq_el0;
    if(!tn)
        return;
    
    cntfrq_el0 = read_sysreg(cntfrq_el0);
    tn->callback = callback;
    tn->data = data;
    tn->time_left = timeval * cntfrq_el0;
    if(tn_insert(tn)){
        // if new t_node is added in the head of list
        timer_set();
        timer_enable();
    }
}
void timer_irq_add(){
    uint32 core0_irq_src = get32(CORE0_IRQ_SOURCE);

    if (core0_irq_src & 0x02){
        timer_disable();
        if(add_task(timer_irq_handler, NULL, TIMER_PRIO))
            timer_enable();
    }
}

void timer_irq_handler(){
    timer_node *tn;
    if(list_empty(&t_meta.lh))
        return;
    tn = list_first_entry(&t_meta.lh, timer_node, lh);
    list_del(&tn->lh);
    update_remain_time();
    timer_set();

    // execute callback function
    (tn->callback)(tn->data);
    // free the entry in the array
    tn_free(tn);

    timer_enable();
}