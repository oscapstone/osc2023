#include "syscall.h"
#include "uart.h"
#include "loader.h"
#include "initrd.h"
#include "mailbox.h"
#include "thread.h"
#include "time.h"

// From switch.S
extern Thread* get_current();

// From exp.S
/// Load register back and eret
extern void load_reg_ret(void);

// From thread.c
extern Thread_q * running, deleted;

int sys_getpid(void){
	Thread* ret = get_current();
	//Log
	/*
	uart_puts("getpid: ");
	uart_puti(ret->id);
	*/
	return ret->id;
}

size_t sys_uart_read(char* buf, size_t size){
	char a;
	char *pivot = buf;
	int count = 0;
	/*
	a = uart_getc();
	while(a != 0){
		*pivot++ = a;
		a = uart_getc();
		count++;
	}
	*pivot = a;
	count ++;
	*/
	for(int i = 0; i < size; i++){
		*pivot++ = uart_getc();
		if(*(pivot- 1) == '\n')
			break;
		count ++;
	}
	*pivot = 0;
	/*
	uart_puts("\n[get]: ");
	uart_puts(buf);
	uart_puti(count);
	uart_puti(size);
	*/
	return count;
}

size_t sys_uart_write(const char* buf, size_t size){
	/*
	uart_puts("\n[write]");
	uart_puth(buf);
	uart_puti(size);
	*/
	const char* t = buf;
	for(int i = 0; i < size; i ++){
		uart_putc(*t++);
	}
	return size;
}

int sys_exec(const char* name, char* const argv[]){
	char* start = (char*)initrd_content_getLo(name);
	int size = initrd_content_getSize(name);
	char* dest = getProgramLo() ; // Displacement
	char* d = dest;
	for(int i = 0; i < size; i ++){
		*d++ = *start++;
	}
	if(start != 0){
		run_program(dest);
		return 0;
	}
	return 1;
}

void sys_exit(int status){
	// Currently not implement the status
	exit();
	return 0;
}

int sys_mbox_call(unsigned char ch, unsigned int *mbox){
	return sys_mailbox_config(ch, mbox);
}


void sys_kill(int pid){
	Thread *t;
	t = thread_q_delete_id(&running, pid);
	if(t == NULL)
		return;
	thread_q_add(&deleted, t);
	return;
}

int sys_fork(Trap_frame* trap_frame){
	Thread* cur = get_current();
	// Child need to load regs from its trap_frame 
	Thread* child = thread_create(load_reg_ret);
	// Copy entire struct from parent -> child
	char *c = (char*) child;
	char *f = (char*) cur;
	// Copy stacks...
	for(int i = sizeof(Thread); i < 0x1000; i ++){
		*(c + i) = *(f + i);
	}
	// Copy callee regs
	for(int i = 0; i < sizeof(callee_regs); i++){
		*(c + i) = *(f + i);
	}

	// Setup Trap_frame of child
	// NOTE: sp always > cur
	/*
	uart_puts("copy trap_frames\n");
	uart_puthl(trap_frame);
	uart_puts(".");
	uart_puthl(cur);
	uart_puts(".");
	uart_puthl(child);
	*/

	child->regs.lr = load_reg_ret;
	cur->regs.sp = trap_frame;
	child->regs.sp = (((void*)trap_frame) - ((void*)cur)
				+ ((void*) child));
	child->regs.fp = (char*)child + 0x1000 - 16;
	// Setup the return value for child
	Trap_frame *trap_frame_child = child->regs.sp;

	trap_frame_child->regs[0] = 0;
	trap_frame_child->regs[29] = child->regs.fp;
	// Get the displacement of userspace stack
	trap_frame_child->sp_el0 = (char*)trap_frame->sp_el0 - (char*)cur->sp_el0 + (char*)child->sp_el0;
	
	// Write child's ID in the x0 of parent
	trap_frame->regs[0] = child->id;	
	cur->child = child->id;

	// Copy user stack
	c  = (char*) trap_frame_child->sp_el0;
	f = (char*) trap_frame->sp_el0;
	/*
	uart_puthl(c);
		uart_puts(".");
	uart_puthl(f);
		uart_puts(".");
	uart_puthl(cur->sp_el0);
		uart_puts(".");
		*/
	while(f != (char*)cur->sp_el0){
		*c++ = *f++;
	}
	*c = *f;


	Trap_frame* trap_child = child->regs.sp;
	/*
	for(int i = 0; i < 35; i ++){
		uart_puthl(trap_frame->regs[i]);
		uart_puts(" vs ");
		uart_puthl(trap_child->regs[i]);
		uart_puts("\n");
	}
	*/
		
	// LOG
	/*
	uart_puts("\n[fork] parent: ");
	uart_puti(cur->id);
	uart_puts(" child:");
	uart_puti(child->id);
	uart_puts("\n");
	*/
	return;
}

void fork_test(){
    //printf("\nFork Test, pid %d\n", get_pid());
    uart_puts("\nFork Test, Pid ");
    uart_puti(get_pid());
    uart_puts("\n");
    int cnt = 1;
    int ret = 0;
    if ((ret = fork()) == 0) { // child
        long long cur_sp;
        asm volatile("mov %0, sp" : "=r"(cur_sp));
        //printf("first child pid: %d, cnt: %d, ptr: %x, sp : %x\n", get_pid(), cnt, &cnt, cur_sp);
	uart_puts("first child pid: ");
	uart_puti(get_pid());
	uart_puts(", cnt: ");
	uart_puti(cnt);
	uart_puts(", ptr: ");
	uart_puthl(&cnt);
	uart_puts(", sp: ");
	uart_puthl(cur_sp);
	uart_puts("\n");
        ++cnt;

        if ((ret = fork()) != 0){
            asm volatile("mov %0, sp" : "=r"(cur_sp));
            //printf("first child pid: %d, cnt: %d, ptr: %x, sp : %x\n", get_pid(), cnt, &cnt, cur_sp);
	    uart_puts("first child pid: ");
	    uart_puti(get_pid());
	    uart_puts(", cnt: ");
	    uart_puti(cnt);
	    uart_puts(", ptr: ");
	    uart_puthl(&cnt);
	    uart_puts(", sp: ");
	    uart_puthl(cur_sp);
	    uart_puts("\n");
        }
        else{
            while (cnt < 5) {
                asm volatile("mov %0, sp" : "=r"(cur_sp));
                //printf("second child pid: %d, cnt: %d, ptr: %x, sp : %x\n", get_pid(), cnt, &cnt, cur_sp);
		uart_puts("second child pid: ");
		uart_puti(get_pid());
		uart_puts(", cnt: ");
		uart_puti(cnt);
		uart_puts(", ptr: ");
		uart_puthl(&cnt);
		uart_puts(", sp: ");
		uart_puthl(cur_sp);
		uart_puts("\n");
                delay(1000000);
                ++cnt;
            }
        }
        uexit();
    }
    else {
        //printf("parent here, pid %d, child %d\n", get_pid(), ret);
	uart_puts(" parent here, pid: ");
	uart_puti(get_pid());
	uart_puts(", child: ");
	uart_puti(ret);
	uart_puts("\n");
    }
    uexit();
}

void check_timer(){
	fork();
	fork();
	fork();
	for(int i = 0; i < 1000000; i++){
		delay(100);
		uart_puts("Thread id: ");
		uart_puti(get_pid());
		uart_puts(", cnt: ");
		uart_puti(i);
		uart_puts("\n");
	}
	uexit();
}

// Syscall function for user
int get_pid(){
	uint32_t ret;
	asm volatile(
		"mov x8, 0;"
		"svc 0;"
		"mov %[ret], x0;"
		:[ret] "=r" (ret):
	);
	return ret;
}

void uexit(){

	asm volatile(
		"mov	x8, 5;"
		"svc	5;"
		::
	);
	return ;
}

int fork(){
	int ret;
	asm volatile(
		"mov	x8, 4;"
		"svc	4;"
		"mov	%[ret] , x0;"
		: [ret] "=r" (ret):
	);
	return ret;
}
