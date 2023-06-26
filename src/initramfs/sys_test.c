#include "syscall.h"

char hello[] = "hello\r\n";

char hex_char[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

unsigned long long write(unsigned int fd, char buf[], unsigned long long len);
unsigned long long write_dec(unsigned long long val);
unsigned long long write_u64(unsigned long long val);
unsigned long long fork();
unsigned long long getpid();
unsigned long long read(char buf[], unsigned long long len);
unsigned long long exec(char name[], char *arg);
unsigned long long dec2buf(unsigned long long val, char *buf);
unsigned long long u642buf(unsigned long long val, char *buf);
    // unsigned long long x = write(buf, 7);
unsigned long long pid;

char buf[20];
int start(){
    unsigned long long pid = getpid();
    write(1, "\nPID=",5);
    write_dec(pid);
    pid = fork();
    write(1, "\nPID=",5);
    write_dec(pid);
    while(1) {
        
    }
    return;
    // unsigned long long pid = getpid();
    // write("\nFork Test, pid ", 16);
    // write_dec(pid);
    // write("\n", 1);
    // int cnt = 1;
    // int ret = 0;
    // if ((ret = fork()) == 0) { // child
    //     long long cur_sp;
    //     asm volatile("mov %0, sp" : "=r"(cur_sp));
    //     write("first child pid: ", 17);
    //     write_dec(getpid());
    //     write(", cnt: ", 6);
    //     write_dec(cnt); 
    //     write(", ptr: ", 7);
    //     write_u64(&cnt);
    //     write(", sp: ", 7);
    //     write_u64(cur_sp);
    //     write("\n", 1);
    //     ++cnt;

    //     if ((ret = fork()) != 0){
    //         asm volatile("mov %0, sp" : "=r"(cur_sp));
    //         write("first child pid: ", 17);
    //         write_dec(getpid());
    //         write(", cnt: ", 6);
    //         write_dec(cnt); 
    //         write(", ptr: ", 7);
    //         write_u64(&cnt);
    //         write(", sp: ", 7);
    //         write_u64(cur_sp);
    //         write("\n", 1);
    //         // printf("first child pid: %d, cnt: %d, ptr: %x, sp : %x\n", get_pid(), cnt, &cnt, cur_sp);
    //     }
    //     else{
    //         while (cnt < 5) {
    //             asm volatile("mov %0, sp" : "=r"(cur_sp));
    //             // printf("second child pid: %d, cnt: %d, ptr: %x, sp : %x\n", get_pid(), cnt, &cnt, cur_sp);
    //             write("Second child pid: ", 17);
    //             write_dec(getpid());
    //             write(", cnt: ", 6);
    //             write_dec(cnt); 
    //             write(", ptr: ", 7);
    //             write_u64(&cnt);
    //             write(", sp: ", 7);
    //             write_u64(cur_sp);
    //             write("\n", 1);
    //             long x = 1000000;
    //             while(x --) {
    //                 asm volatile("nop");
    //             }
    //             ++cnt;
    //         }
    //     }
    //     // exit();
    // }
    // else {
    //     write("parent\n", 7);
    //     // printf("parent here, pid %d, child %d\n", get_pid(), ret);

    // }
    // 
}

unsigned long long dec2buf(unsigned long long val, char *buf) {
    unsigned long long cnt = 0;
    unsigned long long ret = 1;
    while(ret <= val) {
        ret *= 10;
        cnt += 1;
    }
    ret /= 10;
    if(ret == 0) {
        buf[0] = '0';
        return 1;
    }
    for(int i = 0; i < cnt; i ++) {
        buf[i] = val / ret + '0';
        val -= ret;
        ret /= 10;
    }
    return cnt;
}
unsigned long long write(unsigned int fd, char buf[], unsigned long long len){
    asm volatile(
        "mov x8, 13\n"
        "svc 0\n"
    );
    unsigned long long val;
    asm volatile(
        "mov %0, x0\n":"=r"(val)
    );
    return val;
}
unsigned long long read(char buf[], unsigned long long x) {
    asm volatile(
        "mov x8, 1\n"
        "svc 0\n"
    );
    unsigned long long val;
    asm volatile(
        "mov %0, x0\n":"=r"(val)
    );
    return val;
}
unsigned long long fork() {
    asm volatile(
        "mov x8, 4\n"
        "svc 0\n"
    );
    unsigned long long val;
    asm volatile(
        "mov %0, x0\n":"=r"(val)
    );
    return val;
}
unsigned long long getpid() {
    asm volatile(
        "mov x8, 0\n"
        "svc 0\n"
    );
    unsigned long long val;
    asm volatile(
        "mov %0, x0\n":"=r"(val)
    );
    return val;
}
unsigned long long exec(char name[], char *arg) {
    asm volatile(
        "mov x8, 3\n"
        "svc 0\n"
    );
    unsigned long long val;
    asm volatile(
        "mov %0, x0\n":"=r"(val)
    );
    return val;
}

unsigned long long write_dec(unsigned long long val) {

    unsigned long long n = dec2buf(val, buf);
    write(1, buf, n);

    return 0;
}
unsigned long long write_u64(unsigned long long val) {
    for(int cnt = 60; cnt >= 0; cnt -= 4) {
        write(1, &(hex_char[(val >> cnt) & 0xF]), 1);
    }
    return 0;
}