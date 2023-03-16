#include "note.h"
#include "uart.h"
#include "utils.h"
#include "salloc.h"
struct note *note_head;
void init_note()
{
    note_head = NULL;
}

static _list_note(struct note *cur)
{
    if (cur == NULL) return;
    uart_write_string("-------------------------------------\n");
    uart_write_string(cur->content);
    uart_write_string("\n");
    _list_note(cur->next);
}

static void print_note_node(struct note *node)
{
    uart_write_string("-------------------------------------\n");
    uart_write_string(node->content);
    uart_write_string("\n");
}

void list_note()
{
    // _list_note(note_head);
    struct note *cur = note_head;
    while (cur) {
        print_note_node(cur);
        cur = cur->next;
    }
}

void make_note()
{
    char buf[0x100];
    uart_write_string("input note size (note size should within 100 letters): ");
    unsigned int len = uart_read_input(buf, 0x100);
    int note_len = atoi(buf);
    if (note_len > 100 || note_len <= 0) {
        uart_write_string("note length should within 100 characters.\n");
        return;
    }
    struct note *new_note = (struct note *)simple_malloc(sizeof(struct note));
    new_note->content = (char *)simple_malloc((unsigned long long)(note_len+1));
    uart_write_string("input note content: ");
    uart_read_input(new_note->content, note_len+1);
    new_note->next = note_head;
    note_head = new_note;
}

