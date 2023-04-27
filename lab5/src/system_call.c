#include "thread.h"
#include "system_call.h"
#include "check_interrupt.h"
#include "mini_uart.h"
#include "ramdisk.h"
#include "to_EL0.h"
#include "mbox.h"

#define null 0

extern struct thread *get_current();
extern char* ramdisk_start;
extern struct thread *thread_list[100];
extern void to_child();

void EL0_SVC_handler(struct trap_frame *tf)
{
	uint64_t esr_el1;
	asm volatile("mrs %0, ESR_EL1;" : "=r" (esr_el1));
	if(esr_el1>>26 == 0b010101)							//SVC instruction execution in AArch64 state
	{
		switch(tf->reg[8])								//x8 in general register
		{
			case 0:
				tf->reg[0] = getpid();
				break;
			case 1:
				tf->reg[0] = uart_read(tf->reg[0],tf->reg[1]);
				break;
			case 2:
				tf->reg[0] = uart_write(tf->reg[0],tf->reg[1]);
				break;
			case 3:
				char* name = tf->reg[0];
				char* argv = tf->reg[1];
				tf->reg[0] = exec(name,argv);
				break;
			case 4:
				disable_interrupt();
				tf->reg[0] = fork(tf);
				enable_interrupt();
				break;
			case 5:
				exit();								//use previous exit
				break;
			case 6:
				unsigned char ch = tf->reg[0];
				unsigned int *mbox = tf->reg[1];
				tf->reg[0] = mbox_call(mbox,ch);	//use previous mbox_call
				break;
			case 7:
				int pid = tf->reg[0];
				kill(pid);
				break;
			default:
				break;
		}
	}
	return;
}

int getpid()
{
	struct thread *thd = get_current();
	return thd->tid;
}

int uart_read(char* buffer,int size)
{
	int count = 0;
	for(int i=0;i<size;i++)
	{
		*buffer = uart_recv();
		buffer++;
		count++;	
	}
	return count;
}

int uart_write(char* buffer,int size)
{
	int count = 0;
	for(int i=0;i<size;i++)
	{
		uart_send(*(buffer+i));
		count++;
	}
	return count;
}

int exec(char *name,char *argv)
{
	char* prog_start = find_prog(ramdisk_start,name);
	if(prog_start != null)
	{
		int size = find_prog_size(ramdisk_start,name);
		char* start = d_alloc(0x20000 + size);
		char* code = start + 0x10000;						//address to put copy program
		for(int i=0;i<size;i++)
		{
			code[i] = prog_start[i];						//copy
		}
		struct thread *thd = get_current();
		exec_in_EL0(code,(char*)(thd->stack + 0x10000));	//jump to EL0 , bl to program & set SP_EL0 on thread's stack
	}
	return 0;
}

int fork(struct trap_frame *tf)
{
	char *thd = get_current();
	int ctid = Thread(to_child);							//create new child thread
	char *child_copy = thread_list[ctid];					//for child copy
	int gap = (int)child_copy - (int)thd;					//parent & child gap
	for(int i=0;i<sizeof(struct thread) - sizeof(char*);i++)//avoid change kernel_stack pointer
	{
		child_copy[i] = thd[i];								//copy whole thread data to child , include stack
	}

	struct thread *parent = thd;
	for(int i=0;i<0x10000;i++)
	{
		thread_list[ctid]->kernel_stack[i] = parent->kernel_stack[i];	//copy whole kernel_stack
	}

	struct trap_frame *c_tf = (char*)((uint64_t)tf + (uint64_t)thread_list[ctid]->kernel_stack - (uint64_t)parent->kernel_stack);

	char* c_tmp = c_tf;
	char* tmp = tf;
	for(int i=0;i<sizeof(struct trap_frame);i++)
	{
		c_tmp[i] = tmp[i];
	}

	parent->reg.SP = tf;
	thread_list[ctid]->tid = ctid;							//update child tid
	thread_list[ctid]->reg.SP = c_tf;
	thread_list[ctid]->reg.LR = to_child;					//return to child & load register
	thread_list[ctid]->reg.FP = ((uint64_t)thread_list[ctid]->kernel_stack + 0x10000) & 0xFFFFFFF0;
	thread_list[ctid]->next = null;

	//for user_thread
	c_tf->SPSR_EL1 = 0;										//open interrupt & jump back to EL0
	c_tf->reg[0] = 0;										//child's return value
	c_tf->SP_EL0 += gap;									//update sp_el0
	c_tf->reg[29] += gap;

	return ctid;
}

void kill(int pid)
{
	thread_list[pid]->status = "DEAD";
	thread_list[pid] = null;
	return;
}

void fork_test()
{
	uart_send_string("Fork Test, pid ");
	uart_int(test_get_pid());
	uart_send_string("\r\n");
    int cnt = 1;
    int ret = 0;
    if ((ret = test_fork()) == 0) { // child
        long long cur_sp;
        asm volatile("mov %0, sp" : "=r"(cur_sp));
		uart_send_string("first child pid: ");
		uart_int(test_get_pid());
		uart_send_string(", cnt: ");
		uart_int(cnt);
		uart_send_string(", ptr: ");
		uart_hex(&cnt);
		uart_send_string(", sp : ");
		uart_hex(cur_sp);
		uart_send_string("\r\n");
        ++cnt;

        if ((ret = test_fork()) != 0)
		{
            asm volatile("mov %0, sp" : "=r"(cur_sp));
			uart_send_string("first child pid: ");
			uart_int(test_get_pid());
			uart_send_string(", cnt: ");
			uart_int(cnt);
			uart_send_string(", ptr: ");
			uart_hex(&cnt);
			uart_send_string(", sp : ");
			uart_hex(cur_sp);
			uart_send_string("\r\n");
        }
        else
		{
            while (cnt < 5) 
			{
                asm volatile("mov %0, sp" : "=r"(cur_sp));	
				uart_send_string("second child pid: ");
				uart_int(test_get_pid());
				uart_send_string(", cnt: ");
				uart_int(cnt);
				uart_send_string(", ptr: ");
				uart_hex(&cnt);
				uart_send_string(", sp : ");
				uart_hex(cur_sp);
				uart_send_string("\r\n");
				for(int i=0;i<100000000;i++)
				{
					asm volatile("nop;");
				}
                ++cnt;
            }
        }
        test_exit();
    }
    else
	{
		uart_send_string("parent here, pid: ");
		uart_int(test_get_pid());
		uart_send_string(", child ");
		uart_int(ret);
		uart_send_string("\r\n");
    }
	test_exit();
}

int test_get_pid()
{
	int pid;
	asm volatile("mov x8, 0;"
				 "svc 0;"
				 "mov %0, x0;" : "=r" (pid)
				);
	return pid;
}

int test_fork()
{
	int tid;
	asm volatile("mov x8, 4;"
				 "svc 0;"
				 "mov %0, x0;" : "=r" (tid)
				);
	return tid;
}

void test_exit()
{
	asm volatile("mov x8, 5;"
				 "svc 0;"
				);
	return;
}

void test_exec()
{
	asm volatile("mov x8, 3;"
				 "svc 0;"
				);
	return;
}