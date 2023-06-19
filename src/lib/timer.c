#include <timer.h>
#include <utils.h>
#include <mini_uart.h>
#include <syscall.h>
#include <list.h>
#include <irq.h>

#define CORE0_TIMER_IRQ_CTRL 0x40000040
#define CORE0_IRQ_SOURCE 0x40000060
#define TIMER_NUM 32

static void timer_irq_fini();
uint64 timer_boot_cnt;
// static int timer_dump = 0;

timer_node t_nodes[TIMER_NUM];
timer_meta t_meta;

int timer_show_enable;

static void timer_enable(){
    // Enable core0 cntp timer
    put32(PA2VA(CORE0_TIMER_IRQ_CTRL),2);
}

static void timer_disable(){
    // Disable core0 cntp timer
    put32(PA2VA(CORE0_TIMER_IRQ_CTRL),0);
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
    uint32 daif = save_and_disable_interrupt(); 
    timer_node *tn = list_first_entry(&t_meta.lh, timer_node, lh);

    // set timer and metadata
    t_meta.t_interval = tn->time_left;
    write_sysreg(cntp_tval_el0, t_meta.t_interval);

    restore_interrupt(daif);
}

static int tn_insert(timer_node *tn){
    uint32 daif = save_and_disable_interrupt();
    int first;
    timer_node *entry;
    
    update_remain_time();
    first = 1;
    list_for_each_entry(entry, &t_meta.lh, lh){
        if(tn->time_left < entry->time_left)
            break;
        first = 0;
    }
    // insert the t_node to previous of entry
    list_add_tail(&tn->lh,&entry->lh);

    restore_interrupt(daif);
    return first;
}

static timer_node *tn_alloc(){
    uint32 daif, idx;

    // store the interrupt bits
    daif = save_and_disable_interrupt();

    // get the lowest bit idx with value 1
    idx = __builtin_ffs(t_meta.t_status)-1;

    if(idx < 0){
        restore_interrupt(daif);
        return NULL;
    }

    // mask out the bit to 0 because that t_node is in use.
    t_meta.t_status &= ~(1<<idx);
    restore_interrupt(daif);
    return &t_nodes[idx];
}

void timer_init(){
    uint64 cntkctl_el1;
    timer_set_boot_cnt();
    INIT_LIST_HEAD(&t_meta.lh);
    t_meta.size = 0;
    t_meta.t_interval = 0;
    t_meta.t_status = 0xffffffff;
    timer_show_enable = 0;
    cntkctl_el1 = read_sysreg(CNTKCTL_EL1);
    cntkctl_el1 |= 1;
    write_sysreg(CNTKCTL_EL1, cntkctl_el1);
    timer_add_after(boot_time_callback,NULL,2);
}

void boot_time_callback(){
    if(timer_show_enable){
        uint32 cntfrq_el0 = read_sysreg(cntfrq_el0);
        uint64 cntpct_el0 = read_sysreg(cntpct_el0);
        uart_printf("[Boot time: %lld seconds...]\r\n", (cntpct_el0 - timer_boot_cnt) / cntfrq_el0);
    }
    timer_add_after(boot_time_callback, NULL ,2);

}

static void add_timer(timer_node *tn){
    
    if(tn_insert(tn)){
        // if new t_node is added in the head of list
        timer_set();
        timer_enable();
    }
}
int timer_irq_add(){
    uint32 core0_irq_src = get32(PA2VA(CORE0_IRQ_SOURCE));

    if (!(core0_irq_src & 0x02)) {
        return 0;
    }
    
    timer_disable();
    if(irq_add_task(timer_irq_handler, NULL, timer_irq_fini ,TIMER_PRIO))
        timer_enable();
    return 1;
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

void timer_switch_info(){
    timer_show_enable = !timer_show_enable;
}
void timer_add_after(void (*callback)(void *), void *data, uint32 after){
    timer_node *tn;
    uint32 cntfrq_el0;

    tn = tn_alloc();

    if(!tn)
        return;
    
    cntfrq_el0 = read_sysreg(cntfrq_el0);

    tn->callback = callback;
    tn->data = data;
    tn->time_left = after * cntfrq_el0;

    add_timer(tn);
}
// add the timer which tick freq per second
void timer_add_freq(void (*callback)(void *), void *data, uint32 freq)
{
    timer_node *tn;
    uint32 cntfrq_el0;

    tn = tn_alloc();

    if (!tn) {
        return;
    }

    cntfrq_el0 = read_sysreg(cntfrq_el0);

    tn->callback = callback;
    tn->data = data;
    tn->time_left = cntfrq_el0 / freq;
   
    add_timer(tn);
}

static void timer_irq_fini(void){
    timer_enable();
}