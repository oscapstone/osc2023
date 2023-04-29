#include "thread.h"
#include "buddy.h"
#include "system_call.h"
#include "ramdisk.h"
#include "timer.h"

#define null 0

extern void switch_to(struct thread *thd1 , struct thread *thd2);
extern struct thread* get_current();
extern char* ramdisk_start;

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
	for(int i=0;i<65536;i++)
	{
		if(thread_list[i] == null)				//find empty thread
		{
			thd->next = null;
			thd->tid = i;
			thd->reg.x19 = fork_test;
			//thd->reg.x19 = video_prog();										//EL1 switch_to use
			thd->reg.x20 = ((uint64_t)(thd->stack + 0x10000) & 0xFFFFFFF0);		//for user_stack
			thd->kernel_stack = ((uint64_t)(d_alloc(0x10000) + 0x10000) & 0xFFFFFFF0);
			thd->reg.LR = func;
			thd->reg.SP = thd->kernel_stack;									//set EL1 kernel_stack
			thd->reg.FP = thd->kernel_stack;
			thd->status = "RUN";
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
	run_queue = run_queue->next;
	if(now->tid != 0)
	{
		push_run_queue(now);					//may need keep running (RR)
	}
	if(strcmp(next->status,"RUN") == 0)
	{
		switch_to(now,next);
	}
	else if(strcmp(next->status,"DEAD") == 0)
	{
		pop_run_queue(next->tid);
		goto again;
	}
	else
	{
		uart_send_string("status unknow\n");
	}
	return;
}

void kill_zombies()
{
	for(int i=1;i<65536;i++)
	{
		if(thread_list[i] != null && thread_list[i]->status == "DEAD")
		{
			d_free(thread_list[i]->kernel_stack);
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
		uart_send_string("in idel\r\n");
		kill_zombies();
		schedule();
	}
	return;
}

void push_idle()
{
	struct thread *thd;
	thd = d_alloc(sizeof(struct thread));
	thd->next = null;
	thd->tid = 0;
	thd->reg.FP = ((int)(thd->stack + 0x10000) & 0xFFFFFFF0);
	thd->reg.LR = idle;
	thd->reg.SP = ((int)(thd->stack + 0x10000) & 0xFFFFFFF0);
	thd->status = "RUN";
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
			d_free(thread_list[i]->kernel_stack);
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
	tmp->status = "DEAD";
	schedule();
	return;
}

void* video_prog()
{
	char* prog_start = find_prog(ramdisk_start,"syscall.img");
	char* code = null;
	if(prog_start != null)
    {
        int size = find_prog_size(ramdisk_start,"syscall.img");
        code = d_alloc(size);
        for(int i=0;i<size;i++)
        {
            code[i] = prog_start[i];
        }
    }
	void* prog = (void*)code;
	return prog;
}
