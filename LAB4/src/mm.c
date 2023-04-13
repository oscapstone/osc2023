#include "mm.h"
#include "mini_uart.h"
#include "list.h"
#include "utils_c.h"
#include "_cpio.h"
#include "dtb.h"

#define DEBUG

// Exercise
// #define FRAME_BASE ((uintptr_t)0x10000000)
// #define FRAME_END ((uintptr_t)0x20000000)

// Advanced
// get from mailbox's arm memory
#define FRAME_BASE ((uintptr_t)0x0)
// #define FRAME_END ((uintptr_t)0x3b400000)
#define FRAME_END ((uintptr_t)0x3c000000)

#define PAGE_SIZE 0x1000 // 4KB
#define FRAME_ARRAY_SIZE ((FRAME_END - FRAME_BASE) / PAGE_SIZE)

#define FRAME_BINS_SIZE 12
#define MAX_ORDER (FRAME_BINS_SIZE - 1)
#define ORDER2SIZE(order) (PAGE_SIZE * (1 << (order)))
#define FRAME_MAX_SIZE ORDER2SIZE(MAX_ORDER)

#define CHUNK_BINS 7
#define CHUNK_MIN_SIZE 32 // 32Byte - 2KB
#define CHUNK_MAX_ORDER (CHUNK_BINS - 1)

#define FRAME_FREE 0x8      // bit 3
#define FRAME_INUSE 0x4     // bit 2
#define FRAME_MEM_CHUNK 0x2 // bit 1
#define IS_INUSE(frame) (frame.flag & FRAME_INUSE)
#define IS_MEM_CHUNK(frame) (frame.flag & FRAME_MEM_CHUNK)

// for mm_int
extern char _skernel, _ekernel;
extern void *_dtb_ptr;

FrameFlag *frames; // startup時先為整個FRAME_BASE - FRAME_END做array管控的初始化紀錄用
// linking linked list單獨抽象出串列的部分
list frame_bins[FRAME_BINS_SIZE]; // 雙向鏈結串列, 這個free list array裡有好幾條不同量級的free lists
Chunk *chunk_bins[CHUNK_BINS];    // 單向鏈結串列, 這個free chunk array裡有好幾條不同量級的free chunks

unsigned long *smalloc_cur = (unsigned long *)STARTUP_MEM_START;

////////////////////////////////////////////////////////////////////////////////////////////////
//                                          utils                                             //
////////////////////////////////////////////////////////////////////////////////////////////////
// n 值向上對齊到下一個 2 的冪次方。
static unsigned align_up_exp(unsigned n)
{
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n++;
    return n;
}
// 將輸入的 addr 指針轉換為一個索引值
static int addr2idx(void *addr)
{
    return (((uintptr_t)addr & -PAGE_SIZE) - FRAME_BASE) / PAGE_SIZE;
}

////////////////////////////////////////////////////////////////////////////////////////////////
//                                          startUp                                           //
////////////////////////////////////////////////////////////////////////////////////////////////

void *smalloc(size_t size)
{
    align(&size, 4); // allocated the memory size is mutiple of 4 byte;
    unsigned long *smalloc_ret = smalloc_cur;
    if ((uint64_t)smalloc_cur + size > (uint64_t)STARTUP_MEM_END) // range 250MB
    {
        uart_printf("[!] No enough space!\r\n");
        return NULL;
    }
    smalloc_cur += (unsigned int)size; // 寫成smalloc_cur += (unsigned long)size;會怎樣？
    // uart_printf("return addr at %x\n",smalloc_ret);
    return smalloc_ret;
}

////////////////////////////////////////////////////////////////////////////////////////////////
//                                          others                                             //
////////////////////////////////////////////////////////////////////////////////////////////////

void mm_init()
{
    init_buddy();
    memory_reserve((uintptr_t)0, (uintptr_t)0x1000); // Spin tables for multicore boot

    memory_reserve((uintptr_t)&_skernel, (uintptr_t)&_ekernel); // Kernel image in the physical memory

    fdt_traverse(get_initramfs_addr, _dtb_ptr);
    memory_reserve((uintptr_t)initramfs_start, (uintptr_t)initramfs_end); // Kernel image in the physical memory

    memory_reserve((uintptr_t)STARTUP_MEM_START, (uintptr_t)STARTUP_MEM_END); // simple simple_allocator

    memory_reserve(dtb_start, dtb_end); // Devicetree

    merge_useful_pages();

    // 搭建好整個一開始buddy system全都是max order的串列
    // 全都先merge成frame_bins[max_order]
}

void memory_reserve(uintptr_t start, uintptr_t end)
{
    // 將 start 和 end 分別{向下}和{向上對齊}到系統定義的頁框大小，然後對這些頁框進行標記。
    start = start & ~(PAGE_SIZE - 1);
    end = align_up(end, PAGE_SIZE);
    for (uintptr_t i = start; i < end; i += PAGE_SIZE)
    {
        int idx = addr2idx((void *)i);
        frames[idx].flag |= FRAME_INUSE;
        frames[idx].order = 0;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////
//                                   buddy system                                             //
////////////////////////////////////////////////////////////////////////////////////////////////
// 計算需要多少個頁框的大小，回傳該大小所需的位元數，使用內建函數 __builtin_ctz() 找出二進位表示中最後一個 1 的位置，即為該數的位元數。
static int pages_to_frame_order(unsigned pages)
{
    pages = align_up_exp(pages); // 將 pages 轉成最接近且大於 pages 的 2 的冪次方
    return __builtin_ctz(pages);
}

// 初始化 buddy 系統
void init_buddy()
{
    frames = (FrameFlag *)smalloc(sizeof(FrameFlag) * FRAME_ARRAY_SIZE); // 分配記憶體空間給 frames，每個元素表示一個頁框
    for (int i = 0; i < FRAME_BINS_SIZE; i++)                            // 初始化每條free list
    {
        list_init(&frame_bins[i]);
    }
    for (int i = 0; i < FRAME_ARRAY_SIZE; i++) // 初始化每個頁框的狀態
    {
        frames[i].flag = 0;
        frames[i].order = 0;
    }

    // 所以free_list與各頁框狀態frames是分開來維護的,降低彼此之間的耦合度
}

// void merge_useful_pages(): 用於將空閒的頁框按照大小順序合併，以便在分配記憶體時快速找到符合大小要求的頁框。
// static void *split_frames(int order, int target_order): 將一個大小為 2^order 的頁框切割成大小為 2^target_order 的頁框。
// void *alloc_pages(unsigned int pages): 分配指定數量的頁框，並將其標記為已使用。
// void free_pages(void *victim): 釋放已使用的頁框，並將其標記為空閒。

void merge_useful_pages()
{
    for (int order = 0; order < MAX_ORDER; order++)
    {
        int page_idx = 0;
        void *page_addr = 0;
        while (1)
        {
            // 如果這個頁框沒有被使用
            if (!IS_INUSE(frames[page_idx]))
            {
                int buddy_page_idx = page_idx ^ (1 << order);
                // 如果鄰居頁框也沒有被使用，且大小和當前頁框一樣
                if (!IS_INUSE(frames[buddy_page_idx]) &&
                    order == frames[buddy_page_idx].order)
                {
                    // 將當前頁框和鄰居頁框合併成一個更大的頁框，並標記為已使用
                    insert_tail(&frame_bins[order + 1], page_addr);
                    frames[page_idx].order = order + 1;
                }
                else
                {
                    // 如果鄰居頁框不符合合併的條件，則將當前頁框標記為空閒
                    insert_tail(&frame_bins[order], page_addr);
                    frames[page_idx].order = order;
                }
                // LIFO doubly linked list
            }

            // 計算下一個頁框的位置
            page_idx += (1 << (order + 1));
            page_addr = (void *)(FRAME_BASE + page_idx * PAGE_SIZE);

            // (page_idx >= FRAME_ARRAY_SIZE) ? break : continue;

            if (page_idx >= FRAME_ARRAY_SIZE)
            {
                break;
            }
        }
    }
}

// 該函數的參數 order 是要分割的頁框的大小（以頁表指數為單位），而 target_order 是希望獲得的較小頁框的大小。
// 該函數使用 remove_head 從大小為 order 的頁框列表中刪除一個頁框，並使用指標運算將其分成兩個大小為 order-1 的頁框，
// 再使用 insert_head 將其插入到大小為 order-1 的頁框列表中。這樣一直執行到得到大小為 target_order 的頁框，然後將其標記為已使用並返回。
static void *split_frames(int order, int target_order)
{
    list *ptr = remove_head(&frame_bins[order]);

#ifdef DEBUG
    uart_printf("split frame: %x\n", (unsigned long)ptr);
#endif

    for (int i = order; i > target_order; i--) // till i == target_order
    {
        /* insert splitted frame(last half) to bin list */
        list *half_right = (list *)((char *)ptr + ORDER2SIZE(i - 1));
        insert_head(&frame_bins[i - 1], half_right);
        frames[((uintptr_t)half_right - FRAME_BASE) / PAGE_SIZE].order = i - 1;

#ifdef DEBUG
        uart_printf("insert frame at %x\n", (unsigned long)half_right);
#endif
    }

    // 切割成適當需求大小以後
    int idx = addr2idx(ptr);
    frames[idx].order = target_order;
    frames[idx].flag |= FRAME_INUSE;
    return ptr;
}

// 它的參數 pages 是要分配的頁框數量，它使用 pages_to_frame_order 計算所需頁框大小的頁表指數，並檢查該大小的頁框列表中是否有可用的頁框。
// 如果是，則使用 remove_head 從列表中刪除一個頁框，標記為已使用並返回。
// 否則，從較大的頁框開始搜索，將大的頁框分割為較小的頁框，並將其插入到較小的頁框列表中。最後，如果無法找到可用的頁框，則返回 NULL。
void *alloc_pages(unsigned int pages)
{
    int target_order = pages_to_frame_order(pages);
    // 先看看有沒有同量級能分配的，有就直接分配
    if (frame_bins[target_order].next != &frame_bins[target_order])
    {
        list *ptr = remove_head(&frame_bins[target_order]);
#ifdef DEBUG
        uart_printf("return page at: %x\n", (unsigned long)ptr); // 獲分配了哪個page,不必split frame
#endif
        int idx = addr2idx(ptr);
        frames[idx].order = target_order;
        frames[idx].flag |= FRAME_INUSE;
        return ptr;
    }
    // 沒有就進到split_frames階段看看有沒有能拆小來用的
    else
    {
        // 從target_order開始往上找最接近的量級來切分，同減法借位的規則，優先找上一位借位
        for (int i = target_order; i < FRAME_BINS_SIZE; i++)
        {
            if (frame_bins[i].next != &frame_bins[i]) // 代表該量級有得拆
            {
                return split_frames(i, target_order);
            }
        }
    }

    uart_send_string("alloc page return NULL");
    return NULL;
}

// 它的參數 victim 是要釋放的頁框指針，它使用 addr2idx 計算頁框在物理內存中的索引，然後檢查它是否已經被標記為已使用。
// 如果已使用，它將其標記為未使用，然後將其插入到相應大小的頁框列表中。
// 接下來，它將檢查鄰近的同等大小的頁框是否有buddy可以合併。
void free_pages(void *victim)
{
    int page_idx = ((uintptr_t)victim - FRAME_BASE) / PAGE_SIZE;
    if (!IS_INUSE(frames[page_idx]))
    {
        uart_printf("Error! double free the memory at %x", (uintptr_t)victim);
        return;
    }

    unsigned int order = frames[page_idx].order;
    int buddy_page_idx = page_idx ^ (1 << order); // 透過page_idx與2^order做xor找buddy
    frames[page_idx].flag &= ~FRAME_INUSE;        // FRAME_INUSE設成Unuse

    // 假設是放了一個4KB的page，需開始在同個量級找buddy能合併就合併，但也只能跟buddy合併，依序併成maximal order free block
    while (order <= MAX_ORDER &&
           !IS_INUSE(frames[buddy_page_idx]) &&
           order == frames[buddy_page_idx].order)
    {
        void *buddy_victim = (void *)(FRAME_BASE + buddy_page_idx * PAGE_SIZE);
        remove((list *)buddy_victim);

#ifdef DEBUG
        uart_printf("merge buddy frame: %x \n", (unsigned long)buddy_victim);
#endif

        order += 1;
        victim = page_idx < buddy_page_idx ? victim : buddy_victim;
        page_idx = page_idx < buddy_page_idx ? page_idx : buddy_page_idx;
        buddy_page_idx = page_idx ^ (1 << order);
    }

    // while跑完後,merge到盡可能最大量級了
    insert_head(&frame_bins[order], victim);
    frames[page_idx].order = order;

#ifdef DEBUG
    uart_printf("attach frame: %x \n\n", (unsigned long)victim);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////
//                                          chunks                                            //
////////////////////////////////////////////////////////////////////////////////////////////////
// 計算給定大小所對應的 Chunk 的大小級別
static int size_to_chunk_order(unsigned int size)
{
    size = align_up_exp(size); // 將 size 調整為最接近且大於它的 2 的次方數
    size /= CHUNK_MIN_SIZE;    // 將 size 按 CHUNK_MIN_SIZE 進行向下取整的除法

    // 調換順序不知道怎麼樣？？？？？
    //  size /= CHUNK_MIN_SIZE;
    //  size = align_up_exp(size);
    return __builtin_ctz(size); // 返回 size 除以 2 的次方數後的商的二進制表示中末尾 0 的數量（即二進制中最右邊的 1 的位置）
}

// 分配一個大小為 size 的 Chunk 並返回其指標
static void *get_chunk(uint32_t size)
{
    int order = size_to_chunk_order(size); // 計算 Chunk 大小級別

    void *ptr = chunk_bins[order]; // 從指定大小級別的 Chunk 鏈表 {頭部}取出一個 Chunk,因為是singly linked list
    if (ptr)
    {
        chunk_bins[order] = chunk_bins[order]->next; // 將鏈表頭部指標更新為下一個 Chunk
        int idx = addr2idx(ptr);                     // 將指標轉換為對應的page (frame)索引
        frames[idx].ref_count += 1;                  // 增加引用計數

#ifdef DEBUG
        uart_printf("detach chunk at %x\n", (unsigned long)ptr); // 獲分配了哪個chunk
#endif
    }

    return ptr; // 返回 Chunk 的指標
}

// 這段程式碼定義了一個名為 alloc_chunk 的函式，該函式用於將一塊記憶體切分成多個大小相同的塊，並將這些塊插入到對應大小的chunk_bins中。

// 函式有兩個輸入參數，一個是指向要被分割的記憶體區域的指標，另一個是要分割的塊的大小。其中，size 的單位是位元組。

// 首先，count 變數被計算為每個頁的大小除以分割的塊的大小。接著，透過 addr2idx 函式計算出指向該記憶體區域的指標在 frames 陣列中的索引值，並將其設定為 idx 變數。
// 接著，使用 size_to_chunk_order 函式來計算出分割的塊的大小應該放在哪一個 chunk_bins 中，並將結果存入 order 變數。
// 然後，將 frames[idx].flag 的 FRAME_MEM_CHUNK 位元設置為 1，這表示該記憶體區域已經被分割成多個塊了。同時，將 frames[idx].ref_count 設置為 0，表示這些塊還沒被引用。

//
// 最後，使用一個 for 迴圈將每個塊插入到 chunk_bins[order] 串列的開頭。這裡使用指標運算來計算出每個塊的位址，並將其存儲到 ptr 變數中。接著，將 ptr 的 next 指標設置為 chunk_bins[order]，然後將 chunk_bins[order] 設置為 ptr，
// 即將 ptr 插入到 chunk_bins[order] 串列的開頭。
static void alloc_chunk(void *mem, int size)
{
    int count = PAGE_SIZE / size;
    int idx = addr2idx(mem);
    int order = size_to_chunk_order(size);
    frames[idx].flag |= FRAME_MEM_CHUNK;
    frames[idx].ref_count = 0;
    frames[idx].chunk_order = order;

    for (int i = 0; i < count; i++)
    {
        Chunk *ptr = (Chunk *)((uintptr_t)mem + i * size);

        ptr->next = chunk_bins[order]; //????????????
        chunk_bins[order] = ptr;
        // 因為chunk_bins是singly linked list所以create時，也一律從頭插入

#ifdef DEBUG
        uart_printf("insert chunk at %x\n", (unsigned long)ptr);
#endif
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////
//                           dynamic memory simple_allocator                                         //
////////////////////////////////////////////////////////////////////////////////////////////////

void *kmalloc(unsigned int size)
{
    // 將size向上對齊到PAGE_SIZE的倍數，作為分配空間的大小
    unsigned int aligned_page_size = align_up(size, PAGE_SIZE);
    // 如果分配空間大於FRAME_MAX_SIZE就回傳NULL
    if (aligned_page_size > FRAME_MAX_SIZE)
    {
        return NULL;
    }

    // 如果分配空間小於CHUNK_MIN_SIZE，就設為CHUNK_MIN_SIZE
    size = size < CHUNK_MIN_SIZE ? CHUNK_MIN_SIZE : size;
    void *ptr;
    // 如果分配空間小於PAGE_SIZE，就分配一個小塊chunk
    if (align_up_exp(size) < PAGE_SIZE)
    {
        // 將size向上對齊到2的次方，以計算chunk的大小
        size = align_up_exp(size);
        // 從chunk_bins中取出大小符合的chunk
        ptr = get_chunk(size);

        // 如果chunk_bins中沒有符合的chunk，就從物理頁面中分配一個頁面，並將其切割成符合要求的chunk
        // 所以如果有個frame被切成16個256Byte，且都被分配完了
        // Chunk pool在256B量級中就不會有chunk可以拿，此時!ptr == 1，會再另找新page allocate 切出新的16個256Byte並記錄在chunk pool中
        if (!ptr)
        {
            void *mem = alloc_pages(1);
            alloc_chunk(mem, size);
            ptr = get_chunk(size);
        }
    }
    // 如果分配空間大於等於PAGE_SIZE，就直接分配整個頁面
    else
    {
        unsigned int pages = aligned_page_size / PAGE_SIZE;
        ptr = alloc_pages(pages);
    }
    // 回傳分配到的指標
    return ptr;
}

void kfree(void *ptr)
{
    int idx = addr2idx(ptr);
    // 如果索引超出範圍，就輸出錯誤訊息並返回
    if (idx >= FRAME_ARRAY_SIZE)
    {
        uart_send_string("Error! kfree wrong address\n");
        return;
    }

    // 如果是chunk，就將其放回chunk_bins中，並將相應的frame的ref_count減1
    if (IS_MEM_CHUNK(frames[idx]))
    {
        int order = frames[idx].chunk_order;

        ((Chunk *)ptr)->next = chunk_bins[order];
        chunk_bins[order] = ptr; //????????
        // 因為chunk_bins是singly linked list所以回收時，一律從頭插入

        frames[idx].ref_count -= 1;

#ifdef DEBUG
        uart_printf("free chunk at %x\n", (unsigned long)ptr);
#endif
    }
    // 如果是整個頁面，就釋放該頁面
    else
    {
        free_pages(ptr);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////
//                                          test                                              //
////////////////////////////////////////////////////////////////////////////////////////////////

void test_buddy()
{
    int test_size = 5;
    void *a[test_size];
    uart_send_string("\n\n-----  Malloc  -----\n");
    for (int i = 0; i < test_size; i++)
    {
        a[i] = alloc_pages(test_size);
    }
    uart_send_string("\n\n-----  Free  -----\n");
    for (int i = 0; i < test_size; i++)
    {
        free_pages(a[i]);
    }
}

struct test_b
{
    double b1, b2, b3, b4, b5, b6; // 384Byte
};

void test_dynamic_alloc()
{
    uart_send_string("allocate a1\n");
    int *a1 = kmalloc(sizeof(int));
    uart_send_string("allocate a2\n");
    int *a2 = kmalloc(sizeof(int));
    uart_send_string("allocate b\n");
    struct test_b *b = kmalloc(sizeof(struct test_b));

    uart_send_string("free a1\n");
    kfree(a1);
    uart_send_string("free b\n");
    kfree(b);
    uart_send_string("free a2\n");
    kfree(a2);
}
