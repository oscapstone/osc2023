#ifndef __PROCESS_H
#define __PROCESS_H

#include "signal.h"
#include "type.h"
#include "ds/list.h"
#include "thread.h"
#include "interrupt.h"
#include "mmu/mmu.h"
#include "fs/vfs.h"

#define PID_MAX 2048
#define P_FD_MAX 20
typedef enum PStatus_ {
    P_INIT=0,
    P_RUNNING,
    P_WAIT,
    P_EXIT,
    P_ZOMBIE
}PStatus;

// struct ProcMemReg_t{
//     void *addr;
//     uint64_t sz;
//     // struct ds_list_head reg_head;
// };
struct Process_t {
    // uint64_t pid;
    // PStatus status;
    // char *filename;
    // struct ds_list_head pq_head;
    // struct ds_list_head mem_reg_list;
    // pde_t* pgdir; // not used in this lab
    PStatus status;
    uint32_t pid;
    uint32_t exit_code;
    struct Process_t *parent;
    struct ds_list_head th_list;
    struct ds_list_head pl_head;
    // struct File_t *ofile[] // not used in this lab
    // struct inode *cwd; // not used in this lab
    uint8_t signal[10 + 1];
    uint8_t handling_signal;
    struct ds_list_head sig_list;
    void *share_section;
    uint64_t share_section_sz;
    uint64_t ttbr0_el1;
    char name[16];
    struct vnode* cur_vnode;
    struct mount *mnt;
    struct file *files[P_FD_MAX];
};
struct Process_t *process_get_current();
struct Process_t* process_load(const char *filename);
uint64_t process_fork(struct Trapframe_t *frame);
void process_control_init();
uint64_t process_exec(const char* name, char *const argv[], uint8_t kernel);
struct Thread_t* process_thread_create(void * func, void *arg, struct Process_t* proc, uint8_t kernel);
struct Process_t *create_process_instance();
void process_free(struct Process_t *proc);
struct Process_t *get_process_from_pid(uint32_t pid);
void run_user_thread();
#endif