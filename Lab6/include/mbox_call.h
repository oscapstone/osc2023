#ifndef _MBOX_CALL_H
#define _MBOX_CALL_H

/* a properly aligned buffer */
extern volatile unsigned int mbox[36];

int mbox_call(unsigned char ch);

#endif /*_MBOX_CALL_H */
