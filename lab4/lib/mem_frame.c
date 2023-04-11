#include "mini_uart.h"
#include "math.h"
#include "mem_frame.h"
#include "mem_utils.h"

#define NULL 0xFFFFFFFF

struct frame {
        int val;
        struct frame *prev;
        struct frame *next;
};

static int max_block_nframe;
static unsigned int num_frame;

static int logging = 0;
static struct frame *frame_list[MAX_ORDER + 1];
static struct frame *frame_array;

struct reserved_adr {
        unsigned long start;
        unsigned long end;
};

static int finished_reserve;
static int num_reserve;
static struct reserved_adr reserve_list[MAX_RERSERVE];

unsigned int get_frame_id(struct frame *ptr)
{
        return (((unsigned long)ptr - (unsigned long)frame_array) 
                        / sizeof(struct frame));
}

unsigned int get_block_size(int id)
{
        int val = frame_array[id].val;
        if (val < 0) {
                uart_send_string("[ERROR] try to get invalid block size\r\n");
                return 0;
        }
        return (FRAME_SIZE * pow(2, val));
}

void* id_to_physical_address(int id)
{
        return (void*)(id * FRAME_SIZE + MEM_START);
}

unsigned int physical_address_to_id(void* ptr)
{
        return (unsigned int)(((unsigned long)ptr - MEM_START) / FRAME_SIZE);
}

void init_frames(void)
{
        frame_array = simple_malloc(NUM_FRAME * sizeof(struct frame));

        max_block_nframe = (int)pow(2, MAX_ORDER);
        num_frame = NUM_FRAME;

        int last_block_id = 0;
        for (int i = 0; i < NUM_FRAME; i++) {
                if (i % max_block_nframe == 0) {
                        /*
                         * First frame of a block
                         */
                        frame_array[i].val = MAX_ORDER;
                        frame_array[i].prev
                                = &frame_array[i - max_block_nframe];
                        frame_array[i].next
                                = &frame_array[i + max_block_nframe];
                        last_block_id = i;
                } else {
                        /*
                         * Buddy
                         */
                        frame_array[i].val = FSTATE_BUDDY;
                        frame_array[i].prev = 0;
                        frame_array[i].next = 0;
                }
        }
        /*
         * Marks remaining frames as FSTATE_NOT_ALLOCATABLE
         */
        if (num_frame - last_block_id < max_block_nframe) {
                for (int i = last_block_id; i < num_frame; i++) {
                        frame_array[i].val = FSTATE_NOT_ALLOCATABLE;
                        frame_array[i].prev = 0;
                        frame_array[i].next = 0;
                }
                last_block_id -= max_block_nframe;
        }
        frame_array[0].prev = 0;
        frame_array[last_block_id].next = 0;
        /*
         * Initializes frame_list
         */
        for(int i = 0; i < MAX_ORDER; i++) {
                frame_list[i] = 0;
        }
        frame_list[MAX_ORDER] = &frame_array[0];
}

/*
 * Return frame id
 */
unsigned int get_first_frame_from_list(unsigned int order)
{
        unsigned int id = get_frame_id(frame_list[order]);

        frame_list[order] = frame_list[order]->next;
        if (frame_list[order] != 0) frame_list[order]->prev = 0;
        frame_array[id].next = 0;

        if (logging) {
                uart_send_string("[LOG] get_first_frame_from_list: order = ");
                uart_send_int((int)order);
                uart_send_string(", id = ");
                uart_send_int((int)id);
                uart_endl();
        }
        return id;
}

void add_frame_to_list(unsigned int id, unsigned int order)
{
        if (frame_list[order] != 0) {
                frame_list[order]->prev = &frame_array[id];
        }
        frame_array[id].next = frame_list[order];
        frame_list[order] = &frame_array[id];
        frame_array[id].val = order;

        if (logging) {
                uart_send_string("[LOG] add_frame_to_list: order = ");
                uart_send_int((int)order);
                uart_send_string(", id = ");
                uart_send_int((int)id);
                uart_endl();
        }
}

void cut_memory_block(unsigned int id, unsigned int target_order)
{
        if (logging) {
                uart_send_string("[LOG] cut_memory_block: id = ");
                uart_send_int((int)id);
                uart_send_string(", target_order = ");
                uart_send_int((int)target_order);
                uart_endl();
        }

        unsigned int current_order = frame_array[id].val;
        unsigned int current_nframe = pow(2, current_order);

        while (current_order > target_order) {
                current_nframe /= 2;
                current_order -= 1;
                int new_id = id + current_nframe;
                add_frame_to_list(new_id, current_order);
        }
        frame_array[id].val = target_order;
}

void* allocate_frame(unsigned int order)
{
        if (order > MAX_ORDER) {
                uart_send_string("[ERROR] request more than max order\r\n");
                return 0;
        }

        int i;
        for (i = order; frame_list[i] == 0 && i <= MAX_ORDER; i++);
        if (i > MAX_ORDER) {
                uart_send_string("[ERROR] page frame not enough\r\n");
                return 0;
        }

        unsigned int id = get_first_frame_from_list(i);
        if (i > order) cut_memory_block(id, order);
        frame_array[id].val = FSTATE_ALLOCATED;
        return id_to_physical_address(id);
}

/*
 * Returns 0 on failure
 */
int remove_frame_from_list(unsigned int id)
{
        int order = frame_array[id].val;
        if (order < 0) return 0;

        struct frame *prev = frame_array[id].prev;
        struct frame *next = frame_array[id].next;

        if (prev) prev->next = next;
        if (next) next->prev = prev;
        frame_array[id].prev = 0;
        frame_array[id].next = 0;

        if (frame_list[order] == &frame_array[id]) frame_list[order] = next;

        if (logging) {
                uart_send_string("[LOG] remove_frame_from_list: order = ");
                uart_send_int((int)order);
                uart_send_string(", id = ");
                uart_send_int((int)id);
                uart_endl();
        }
        return 1;
}

void merge_with_buddy(unsigned int id)
{
        int i;
        for (i = 0; i < MAX_ORDER; i++) {
                unsigned int buddy_id = id ^ (1 << i);
                if (frame_array[buddy_id].val == FSTATE_BUDDY) continue;
                if (frame_array[buddy_id].val == FSTATE_ALLOCATED) break;
                if (frame_array[buddy_id].val == FSTATE_NOT_ALLOCATABLE) break;
                if (frame_array[buddy_id].val != i) {
                        uart_send_string("[ERROR] wrong value QQ\r\n");
                        break;
                }
                remove_frame_from_list(buddy_id);
                frame_array[buddy_id].val = FSTATE_BUDDY;
                id &= ~(1 << i);
        }
        add_frame_to_list(id, i);
}

void free_frame(void *ptr)
{
        unsigned int id = physical_address_to_id(ptr);
        if (id < 0 || id >= num_frame) {
                uart_send_string("[ERROR] try to free frame out of space, id ");
                uart_send_int(id);
                uart_endl();
                return;
        }
        merge_with_buddy(id);
}

void memory_reserve(void* start, void* end)
{
        if (finished_reserve) {
                uart_send_string("[ERROR] memory reservation already finished");
                return;
        }
        if (num_reserve >= MAX_RERSERVE) {
                uart_send_string("[ERROR] exceed max number of reseravtion");
                return;
        }
        reserve_list[num_reserve].start = (unsigned long)start;
        reserve_list[num_reserve].end = (unsigned long)end;
        num_reserve++;
}

void merge_overlap_reservation(int i, int j)
{
        if (reserve_list[i].start == reserve_list[j].end) {
                reserve_list[i].start = reserve_list[j].start;
        } else if (reserve_list[i].end == reserve_list[j].start) {
                reserve_list[i].end = reserve_list[j].end;
        } else if (reserve_list[i].start <= reserve_list[j].start
                        && reserve_list[i].end >= reserve_list[j].end) {
        } else if (reserve_list[i].start >= reserve_list[j].start
                        && reserve_list[i].end <= reserve_list[j].end) {
                reserve_list[i].end = reserve_list[j].end;
                reserve_list[i].start = reserve_list[j].start;
        } else {
                return;
        }
        reserve_list[j].start = NULL;
        reserve_list[j].end = NULL;
}

int clean_reservation_list(void) {
        int cnt = 0;
        for (int i = 0; i < num_reserve; i++) {
                if (reserve_list[i].start == NULL) continue;
                reserve_list[cnt].start = reserve_list[i].start;
                reserve_list[cnt].end = reserve_list[i].end;
                cnt ++;
        }
        return cnt;
}

void turn_reserve_adr_to_id(void)
{
        for (int i = 0; i < num_reserve; i++) {
                reserve_list[i].start = physical_address_to_id(
                                        (void*)reserve_list[i].start);
                reserve_list[i].end = physical_address_to_id(
                                        (void*)(reserve_list[i].end - 1));

                if (reserve_list[i].start > reserve_list[i].end) {
                        reserve_list[i].start = NULL;
                        reserve_list[i].end = NULL;
                        uart_send_string(
                                "[ERROR] invalid reservation address\r\n");
                        continue;
                }
                for (int j = 0; j < i; j++) {
                        merge_overlap_reservation(i, j);
                }
        }
        num_reserve = clean_reservation_list();
        if (logging) {
                uart_send_string("[LOG] parts to reserve: ");
                for (int i = 0; i < num_reserve; i++) {
                        uart_send_string(" (");
                        uart_send_int(reserve_list[i].start);
                        uart_send_string(", ");
                        uart_send_int(reserve_list[i].end);
                        uart_send_string(") ");
                        uart_endl();
                }
        }
}

void allocate_id_order(unsigned int id, int order)
{
        if (logging) {
                uart_send_string("[LOG] allocate_id_order: id = ");
                uart_send_int(id);
                uart_send_string(", order = ");
                uart_send_int(order);
                uart_endl();
        }

        unsigned int block_id = id;
        unsigned int block_order = order;
        while (!remove_frame_from_list(block_id)) {
                block_id &= ~(1 << block_order);
                block_order++;
        }
        block_order = frame_array[block_id].val;

        if (logging) {
                uart_send_string("[LOG] found alocatable: id = ");
                uart_send_int(block_id);
                uart_send_string(", order = ");
                uart_send_int(block_order);
                uart_endl();
        }

        while (block_order > order) {
                if (block_id == id) {
                        cut_memory_block(id, order);
                        break;
                }
                block_order --;
                cut_memory_block(block_id, block_order);
                /*
                 * Moves if request frame is at the right side
                 */
                if ((block_id | (1 << block_order)) <= id) {
                        add_frame_to_list(block_id, block_order);
                        block_id = block_id | (1 << block_order);
                        remove_frame_from_list(block_id);
                        frame_array[block_id].val = block_order;
                }
        }
        frame_array[id].val = FSTATE_ALLOCATED;
}

void allocate_id_range(unsigned int start, unsigned int end, int order)
{
        if (start == NULL || end == NULL) return;
        if (logging) {
                uart_send_string("[LOG] allocate_id_range: start = ");
                uart_send_int(start);
                uart_send_string(", end = ");
                uart_send_int(end);
                uart_send_string(", order = ");
                uart_send_int(order);
                uart_endl();
        }
        /*
         * Break down if requested size > size of MAX_ORDER blocks
         */
        if ((order == MAX_ORDER) && ((end - start + 1) >= max_block_nframe)) {
                unsigned int head_id = start;
                /*
                 * Allocates the prefix part that is smaller than MAX_ORDER
                 */
                if (head_id % max_block_nframe != 0) {
                        head_id >>= order;
                        head_id += 1;
                        head_id <<= order;
                        allocate_id_range(start, head_id - 1, order - 1);
                }
                /*
                 * Allocates the whole block of MAX_ORDER
                 */
                while (head_id + max_block_nframe <= (end + 1)) {
                        allocate_id_order(head_id, order);
                        head_id += max_block_nframe;
                }
                /*
                 * Allocates the remaining part
                 */
                if (head_id <= end) allocate_id_range(head_id, end, order - 1);
                return;
        }
        /*
         * Allocates if requested range fits the size of current order
         */
        if (start % (1 << order) == 0 && (end + 1) % (1 << order) == 0) {
                allocate_id_order(start, order);
                return;
        }
        /*
         * Allocates block of a smaller order
         */
        order -= 1;
        unsigned int second_id = start;
        second_id >>= order;
        second_id += 1;
        second_id <<= order;
        if (start >= second_id || end < second_id) {
                allocate_id_range(start, end, order);
                return;
        }
        /*
         * Breaks down if it lays across to blocks of current order
         */
        allocate_id_range(start, second_id - 1, order);
        allocate_id_range(second_id, end, order);
}

void process_mem_reserve(void)
{
        finished_reserve = 1;
        turn_reserve_adr_to_id();
        for (int i = 0; i < num_reserve; i++) {
                if (logging) {
                        uart_send_string("[LOG] reserve part\r\n");
                }
                allocate_id_range(
                        reserve_list[i].start, reserve_list[i].end, MAX_ORDER);
        }
}

void demo_frame(void)
{
        logging = 1;

        uart_send_string("demo_frame ------------------------------------\r\n");

        uart_send_string("TEST-1 ALLOCATE FRAME ORDER 0\r\n");
        int* pos = allocate_frame(0);
        uart_send_string("allocated address = ");
        uart_send_hex_64((unsigned long)pos);
        uart_endl();
        uart_endl();

        uart_send_string("TEST-2 ALLOCATE FRAME ORDER 4\r\n");
        int* tmp = allocate_frame(4);
        uart_send_string("allocated address = ");
        uart_send_hex_64((unsigned long)tmp);
        uart_endl();
        uart_endl();

        uart_send_string("TEST-3 FREE FRAME ORDER 0\r\n");
        free_frame(pos);
        uart_endl();

        uart_send_string("-----------------------------------------------\r\n");

        logging = 0;
}