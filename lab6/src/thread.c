#include "thread.h"
#include "buddy.h"
#include "system_call.h"
#include "ramdisk.h"
#include "timer.h"
#include "virtual_mem.h"
#include "map_kernel.h"

#define null 0

extern void switch_to(struct thread *thd1 , struct thread *thd2);
extern struct thread* get_current();
extern char* ramdisk_start;
extern void vm_switch(uint64_t PGD);

struct thread *thread_list[65536];
struct thread *run_queue = null;

void init_thread()
{
	clear_thread_list();
	run_queue = null;
	struct thread *for_init = d_alloc(sizeof(struct thread));
	asm volatile("msr tpidr_el1, %0;" : "=r" (for_init));
	return;
}

int Thread(void (*func)())
{
	struct thread *thd = d_alloc(sizeof(struct thread));
	thd->PGD = (uint64_t)alloc_page_table();		//create a PGD
/*
	uart_hex_64(thd->PGD);
	uart_send_string(" -> PGD\n");
*/
	for(int i=0;i<65536;i++)
	{
		if(thread_list[i] == null)					//find empty thread
		{
			thd->next = null;
			thd->tid = i;

			uint64_t tmp_user_stack_base = d_alloc(0x10000);
			mappage(thd->PGD,0xFFFFFFFFB000,0x4000,tmp_user_stack_base);		//map user stack

			thd->user_stack_base = 0xFFFFFFFFB000;	//stack region in SPEC
			thd->user_stack = 0xFFFFFFFFEFFC;
			//thd->reg.x19 = fork_test;				//for fork_test use
			thd->reg.x19 = 0x0;						//had map video() in 0x0
			thd->reg.x20 = thd->user_stack;			//for user_stack
			thd->kernel_stack_base = d_alloc(0x10000);
/*
			uart_hex_64(thd->kernel_stack_base);
			uart_send_string(" -> kernel base\n");
*/
			thd->kernel_stack = ((uint64_t)(thd->kernel_stack_base + 0x10000) & 0xFFFFFFFFFFFFFFF0);
			thd->reg.LR = func;
			thd->reg.SP = thd->kernel_stack;		//set EL1 kernel_stack
			thd->reg.FP = thd->kernel_stack_base;
			thd->status = 1;
			thd->sig = 0;							//no signal
			for(int i=0;i<10;i++)
			{
				thd->sig_handler[i] = null;
			}
			thread_list[i] = thd;
			break;
		}
	}
	push_run_queue(thd);
	return thd->tid;
}

void push_run_queue(struct thread *thd)
{
	if(run_queue == null)
	{
		run_queue = thd;
	}
	else
	{
		struct thread *tmp = run_queue;
		while(tmp->next != null)
		{
			tmp = tmp->next;
		}
		tmp->next = thd;
	}
	thd->next = null;
	return;
}

void pop_run_queue(int tid)
{
	struct thread *tmp = run_queue;
	if(tmp != null && tmp->tid == tid)				//pop position on head
	{
		run_queue = run_queue->next;
	}
	else if(tmp != null)
	{
		while(tmp->next != null)
		{
			if(tmp->next->tid == tid)
			{
				tmp->next = tmp->next->next;		//pop positoin on body
				break;
			}
			tmp = tmp->next;
		}
	}
	return;
}

void schedule()
{
again:
	if(run_queue == null)
	{
		repush_idle();							//keep running idle
	}
	struct thread *now = get_current();
	struct thread *next = run_queue;
/*
	uart_send_string("now id :");
	uart_int(now->tid);
	uart_send_string(", next id :");
	uart_int(next->tid);
	uart_send_string("\r\n");
*/
	run_queue = run_queue->next;
	if(now->tid != 0)
	{
		push_run_queue(now);					//may need keep running (RR)
	}
	if(next->status == 1)						//1 : RUN
	{
		vm_switch(next->PGD);
		switch_to(now,next);
	}
	else if(next->status == 2)					//2 : DEAD
	{
		pop_run_queue(next->tid);
		goto again;
	}
	else
	{
		uart_send_string("status unknow , id: ");
		uart_int(next->tid);
		uart_send_string("\r\n");
	}
	return;
}

void kill_zombies()
{
	for(int i=1;i<65536;i++)
	{
		if(thread_list[i] != null && thread_list[i]->status == 2)
		{
			d_free(thread_list[i]->kernel_stack_base);
			d_free(thread_list[i]);
			thread_list[i] = null;
		}
	}
	return;
}

void idle()
{
	while(1)
	{
		//uart_send_string("in idel\r\n");
		kill_zombies();
		schedule();
	}
	return;
}

void push_idle()
{
	struct thread *thd = d_alloc(sizeof(struct thread));
	thd->PGD = (uint64_t)alloc_page_table() & ~(0xFFFF000000000000);	//create a PGD (recognized in EL0)
	thd->next = null;
	thd->tid = 0;
	thd->kernel_stack_base = d_alloc(0x10000);
	thd->kernel_stack = (thd->kernel_stack_base + 0x10000);
	thd->reg.FP = thd->kernel_stack_base;
	thd->reg.LR = idle;
	thd->reg.SP = thd->kernel_stack;
	thd->status = 1;
	thread_list[0] = thd;					//always set idle thread in list's first
	push_run_queue(thread_list[0]);
	return;
}

void repush_idle()
{
	push_run_queue(thread_list[0]);
	return;
}

int get_tid()
{
	struct thread *thd = get_current();
	return thd->tid;
}

void clear_thread_list()
{
	for(int i=0;i<65536;i++)
	{
		if(thread_list[i] != null)
		{
			d_free(thread_list[i]->kernel_stack_base);
			d_free(thread_list[i]);
			thread_list[i] = null;
		}
	}
	return;
}

void foo()
{
    for(int i=0;i<10;i++)
    {
        uart_send_string("Thread id: ");
        uart_int(get_tid());
        uart_send_string(" ");
        uart_int(i);
        uart_send_string("\r\n");
        for(int i=0;i<200000000;i++)
        {
            asm volatile("nop");
        }
        schedule();
    }
    exit();
    return;
}

void exit()				//end of a thread
{
	struct thread *tmp = get_current();
	tmp->status = 2;
	schedule();
	return;
}

void video_prog()
{
	char* prog_start = find_prog(ramdisk_start,"vm.img");
	char* code = null;
	int size = 0;
	if(prog_start != null)
    {
        size = find_prog_size(ramdisk_start,"vm.img");
        code = d_alloc(size);
        for(int i=0;i<size;i++)
        {
            code[i] = prog_start[i];
        }
    }
	void* prog = (void*)code;
	struct thread *thd = thread_list[1];

	uart_hex_64(thd->PGD);
	uart_send_string(" -> video\n");

	mappage(thd->PGD,0x0,size,prog);
	return;
}
