#ifndef	_NOTE_H
#define	_NOTE_H
struct note {
    unsigned long long buffer_size;
    char *content;
    struct note *next;
};
extern struct note *note_head;
extern void make_note();
extern void list_note();
extern void init_note();
#endif