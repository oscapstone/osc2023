#include "mem_allocator.h"
#include "mem_frame.h"
#include "mini_uart.h"

struct page_header {
        unsigned int table;
        char *base_address;
        struct page_header *prev;
        struct page_header *next;
};

static int logging;
static struct page_header *pool_head, *pool_tail;
static int num_chunks[NUM_CHUNK_SIZE] = {NUM_CHUNK_32, NUM_CHUNK_64, 
                NUM_CHUNK_128, NUM_CHUNK_256, NUM_CHUNK_512, NUM_CHUNK_1024};
static int chunk_table_offs[NUM_CHUNK_SIZE];
static int chunk_adr_offs[NUM_CHUNK_SIZE];

void init_allocator(void)
{
        int off = PAGE_HEADER_SIZE;
        int num = 0;
        for (int i = 0; i < NUM_CHUNK_SIZE; i++) {
                chunk_adr_offs[i] = off;
                chunk_table_offs[i] = num;
                off += num_chunks[i] * (1 << (i + MIN_CHUNK_ORDER));
                num += num_chunks[i];
        }
        pool_head = 0;
}

struct page_header* add_new_frame_to_pool(void)
{
        struct page_header* page = allocate_frame(0);
        page -> table = 0;
        page -> base_address = (char*)page;
        page -> next = 0;
        page -> prev = 0;

        if (pool_tail) {
                pool_tail -> next = page;
                page -> prev = pool_tail;
        }
        pool_tail = page;
        if (!pool_head) pool_head = page;

        return page;
}

void* get_chunk_for_size(unsigned int size)
{
        int order;
        for (order = 0; (1 << (order + MIN_CHUNK_ORDER)) < size; order++) ;
        if (logging) {
                uart_send_string("[LOG] get_chunk_for_size, size = ");
                uart_send_int(1 << (order + MIN_CHUNK_ORDER));
                uart_endl();
        }
        /*
         * Iterates through pool and looks for a chunk that fit the size
         */
        struct page_header* current_page = pool_head;
        int table_off = chunk_table_offs[order];
        while (current_page != 0) {
                for (int i = 0; i < num_chunks[order]; i++) {
                        unsigned int table_mask = (1 << (table_off + i));
                        if ((current_page->table) & table_mask) continue;
                        /*
                         * Found a space
                         */
                        current_page->table |= table_mask;
                        if (logging)
                                uart_send_string("[LOG] found empty chunk\r\n");
                        return (void*)((current_page->base_address) 
                                + chunk_adr_offs[order] 
                                + i * (1 << (order + MIN_CHUNK_ORDER)));
                }
                current_page = current_page->next;
        }
        /*
         * No empty chunk of requested size
         */
        current_page = add_new_frame_to_pool();
        current_page->table |= (1 << table_off);
        if (logging) {
                uart_send_string("[LOG] allocate a new frame, address = ");
                uart_send_hex_64((unsigned long)current_page);
                uart_endl();
        }
        return (void*)((current_page->base_address) + chunk_adr_offs[order]);
}

void* malloc(unsigned int size)
{
        if (size <= MAX_CHUNK_SIZE) return get_chunk_for_size(size);

        for (int i = 0; i < MAX_ORDER; i++) {
                if (FRAME_SIZE * (1 << i) < size) continue;
                if (logging) {
                        uart_send_string("[LOG] new frame order = ");
                        uart_send_int(i);
                        uart_endl();
                }
                return allocate_frame(i);
        }
        uart_send_string("[ERROR] malloc exceed max size\r\n");
        return 0;
}

void free(void* ptr)
{
        // TODO: case that it is bigger than a FRAME_SIZE???????

        struct page_header* page =
                (struct page_header*)((unsigned long)ptr & FRAME_ADDRESS_MASK);
        unsigned int chunk_off = (unsigned long)ptr - (unsigned long)page;

        int order;
        for (order = 0; chunk_off > chunk_adr_offs[order]; order++) ;
        int id = (chunk_off - chunk_adr_offs[order]) 
                        / (1 << (order + MIN_CHUNK_ORDER));

        page->table &= ~(1 << (chunk_table_offs[order] + id));
        if (logging) {
                uart_send_string("[LOG] free, page address = ");
                uart_send_hex_64((unsigned long)page);
                uart_send_string(", size = ");
                uart_send_int(1 << (order + MIN_CHUNK_ORDER));
                uart_endl();
        }
        if (page->table) return;

        pool_tail = page->prev;
        if (pool_tail) pool_tail->next = 0;
        else pool_head = 0;
        free_frame(page);
        if (logging) uart_send_string("[LOG] free this frame\r\n");
}

void demo_dynamic_allocation(void)
{
        logging = 1;

        uart_endl();
        uart_send_string("demo_malloc -----------------------------------\r\n");
        uart_endl();

        uart_send_string("TEST-1 FIRST ALLOCATION\r\n");
        uart_send_string("# size = 50\r\n");
        char* ptr1 = malloc(50);
        uart_send_string("-> allocated address = ");
        uart_send_hex_64((unsigned long)ptr1);
        uart_endl();
        uart_endl();

        uart_send_string("TEST-2 ALLOCATE FROM EXISTING FRAME\r\n");
        uart_send_string("# size = 64 (Chunk A), address diff = 64 (0x40)\r\n");
        char* to_free_a = malloc(64);
        uart_send_string("-> allocated address = ");
        uart_send_hex_64((unsigned long)to_free_a);
        uart_endl();
        uart_endl();

        uart_send_string("TEST-3 ALLOCATE FROM EXISTING FRAME\r\n");
        uart_send_string("# size = 120\r\n");
        char* ptr2 = malloc(120);
        uart_send_string("-> allocated address = ");
        uart_send_hex_64((unsigned long)ptr2);
        uart_endl();
        uart_endl();

        uart_send_string("TEST-4 ALLOCATE NEW FRAME\r\n");
        uart_send_string("# size = 128 (Chunk B)\r\n");
        char* to_free_b = malloc(128);
        uart_send_string("-> allocated address = ");
        uart_send_hex_64((unsigned long)to_free_b);
        uart_endl();
        uart_endl();

        uart_send_string("TEST-5 FREE\r\n");
        uart_send_string("# size = 128 (Chunk B)\r\n");
        free(to_free_b);
        uart_endl();

        uart_send_string("TEST-6 FREE FRAME\r\n");
        uart_send_string("# size = 64 (Chunk A)\r\n");
        free(to_free_a);
        uart_endl();

        uart_send_string("TEST-7 ALLOCATE NEW FRAME\r\n");
        uart_send_string("# size = 100\r\n");
        char* ptr3 = malloc(100);
        uart_send_string("-> allocated address = ");
        uart_send_hex_64((unsigned long)ptr3);
        uart_endl();

        uart_endl();
        uart_send_string("-----------------------------------------------\r\n");
        uart_endl();

        logging = 0;

        free(ptr1);
        free(ptr2);
        free(ptr3);
}