#ifndef MAILBOX_H
#define MAILBOX_H

int mailbox_call(unsigned char ch, volatile unsigned int *mbox);

void get_board_revision(void);
void get_arm_memory(void);

#endif /* MAILBOX_H */