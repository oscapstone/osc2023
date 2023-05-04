#include "command.h"
#include "heap.h"
#include "initrd.h"
#include "loader.h"
#include "mailbox.h"
#include "mem.h"
#include "str.h"
#include "terminal.h"
#include "thread.h"
#include "timer.h"
#include "uart.h"
#include "vm.h"
// static char buf[256];

struct command commands[] = {
    {
        .name = "help",
        .help = "Show help message!\n",
        .func = help,
    },
    {
        .name = "lshw",
        .help = "Show some HW informations\n",
        .func = lshw,
    },
    {
        .name = "hello",
        .help = "Print \'hello world\'\n",
        .func = hello,
    },
    {
        .name = "reboot",
        .help = "Reboot the device.\n",
        .func = reboot,
    },
    {
        .name = "ls",
        .help = "List file name in FS.\n",
        .func = ls,
    },
    {
        .name = "cat",
        .help = "Show the content of target file.\n",
        .func = cat,
    },
    {
        .name = "run",
        .help = "Run the specific user program.\n",
        .func = run_loader,
    },
    {
        .name = "set timer",
        .help = "set a timer and show the message.\n",
        .func = time_out,
    },
    {
        .name = "async_read",
        .help = "Read by interrupt handler.\n",
        .func = async_read,
    },
    {
        .name = "pmalloc",
        .help = "Alloc a memory from buddy system.\n",
        .func = pmalloc_command,
    },
    {
        .name = "pfree",
        .help = "Free the memory from buddy system.\n",
        .func = pfree_command,
    },
    {
        .name = "smalloc",
        .help = "Alloc a small memory from buddy system.\n",
        .func = smalloc_command,
    },
    {
        .name = "exec",
        .help = "Execute program by thread.\n",
        .func = exec,
    },
    {
        .name = "getpid",
        .help = "get Pid\n",
        .func = getpid_command,
    },

    // ALWAYS The last item of the array!!!
    {
        .name = "NULL", // The end of the array
    }};

int smalloc_command() {
  char *buf = (char *)malloc(sizeof(char) * 256);
  memset(buf, 0, sizeof(char) * 256);
  int size = 0;
  int opt = 0;
  uart_puts("Please input the memroy size: ");
  uart_gets(buf);
  for (int i = 0; i < 10; i++) {
    if (buf[i] >= '0' && buf[i] <= '9') {
      size += buf[i] - '0';
      size *= 10;
    }
  }
  size /= 10;
  if (size == 0) {
    return 0;
  }
  uart_puts("calling smalloc(");
  uart_puti(size);
  uart_puts(")\n");
  uart_puth(smalloc(size));
  return 0;
}

int getpid_command() {
  uart_puts("getPid :");
  uart_puti(sys_getpid());
  uart_puts("\n");
  return 0;
}

int pfree_command() {
  char *buf = (char *)malloc(sizeof(char) * 256);
  memset(buf, 0, sizeof(char) * 256);
  uint64_t size = 0;
  uart_puts("Please input the memroy address to FREE: ");
  uart_gets(buf);
  for (int i = 0; i < 10; i++) {
    if (buf[i] >= '0' && buf[i] <= '9') {
      size += buf[i] - '0';
      size *= 16;
    }
    if (buf[i] >= 'A' && buf[i] <= 'F') {
      size += buf[i] - 'A' + 10;
      size *= 16;
    }
  }
  size /= 16;
  if (size == 0) {
    return 0;
  }
  uart_puts("calling pfree(");
  uart_puth(size);
  uart_puts(")\n");
  pfree(size);
  return 0;
}

int pmalloc_command() {
  char *buf = (char *)malloc(sizeof(char) * 256);
  memset(buf, 0, sizeof(char) * 256);
  int size = 0;
  int opt = 0;
  uart_puts("Please input the memroy size: ");
  uart_gets(buf);
  for (int i = 0; i < 10; i++) {
    if (buf[i] >= '0' && buf[i] <= '9') {
      size += buf[i] - '0';
      size *= 10;
    }
  }
  size /= 10;
  if (size == 0) {
    return 0;
  }
  size /= FRAME_SIZE; // 0x1000
  for (; size > 0; size /= 2) {
    opt++;
  }
  uart_puts("calling pmalloc(");
  uart_puti(opt);
  uart_puts(")\n");
  pmalloc(opt);
  return 0;
}

int async_read() {
  uart_puts("Async Read start...\n");
  enable_uart_receive_int();
  set_timer_read();
  delay(16000000);
  return 0;
}
int time_out() {
  char *buf = (char *)malloc(sizeof(char) * 256);
  char integer[10];
  int second = 0;
  memset(buf, 0, 256);
  memset(integer, 0, 10);
  uart_puts("Message:\n");
  uart_gets(buf);
  uart_puts("Seconds:\n");
  uart_gets(integer);
  // char array -> int
  for (int i = 0; i < 10; i++) {
    if (integer[i] >= '0' && integer[i] <= '9') {
      second += integer[i] - '0';
      second *= 10;
    }
  }
  second /= 10;
  set_timeout(buf, second);
  uart_puts("done\n");

  return 1;
}

int run_loader() {
  char buf[256];
  memset(buf, 0, 256);
  void *start = 0;
  uart_puts("Name:\n");
  uart_gets(buf);
  start = initrd_content_getLo(buf);
  if (start != 0) {
    // uart_puth(start);
    // uart_puth(*(int*)start);
    run_program(start);
    return 0;
  }
  return 1;
}

int exec() {
  char buf[256];
  memset(buf, 0, 256);
  char *start = 0;
  uart_puts("Name:\n");
  uart_gets(buf);
  start = (char *)initrd_content_getLo(buf);
  int size = initrd_content_getSize(buf);
  char *dest = (char *)pmalloc(6); // Get the largest size
  char *d = dest;
  // Copy.
  // If no copy, the adrp instruction in user program will wierd.
  for (int i = 0; i < size; i++) {
    *d++ = *start++;
  }

  if (size != 0) {
    setup_program_loc(dest);
    Thread *user = thread_create(sys_run_program);
    map_vm(user->pgd, 0, dest, 64);
    core_timer_enable();
    schedule();
    Thread *t = get_current();
    t->status = wait;
    while (1) {
      idle();
    }
    return 0;
  }
  return 1;
}

int help() {
  int i = 0;
  while (1) {
    if (!strcmp(commands[i].name, "NULL")) {
      break;
    }
    uart_puts(commands[i].name);
    uart_puts(": ");
    uart_puts(commands[i].help);
    i++;
  }
  return 0;
}

int cat() {
  char buf[256];
  memset(buf, 0, 256);
  uart_puts("Name:\n");
  uart_gets(buf);
  initrd_cat(buf);
  // initrd_cat("boot.S");
  return 0;
}

int ls() {
  initrd_list();
  return 0;
}

int hello() {
  uart_puts("Hello World!\n");
  return 0;
}

int invalid_command(const char *s) {
  uart_putc('`');
  uart_puts(s);
  uart_putc('`');
  uart_puts(" is invalid command! Please use `help` to list commands\n");
  return 0;
}

int lshw(void) {
  uart_puts("Board version\t: ");
  mbox[0] = 7 * 4;
  mbox[1] = MAILBOX_REQ;
  mbox[2] = TAG_BOARD_VER;
  mbox[3] = 4;
  mbox[4] = 0;
  mbox[5] = 0;
  mbox[6] = TAG_LAST;

  if (mailbox_config(CHANNEL_PT)) {
    uart_puth(mbox[5]);
  }
  uart_puts("\nDevice base Mem Addr\t: ");
  mbox[0] = 8 * 4;
  mbox[1] = MAILBOX_REQ;
  mbox[2] = TAG_ARM_MEM;
  mbox[3] = 8;
  mbox[4] = 0;
  mbox[5] = 0;
  mbox[6] = 0;
  mbox[7] = TAG_LAST;
  if (mailbox_config(CHANNEL_PT)) {
    uart_puth(mbox[5]);
    uart_puts("\nDevice Mem size\t: ");
    uart_puth(mbox[6]);
  }
  uart_putc('\n');
  return 0;
}

int reboot() {
  *PM_RSTC = PM_PASSWORD | 0x20; // Reset
  *PM_WDOG = PM_PASSWORD | 180;
  return 0;
}
