void init_buddy();
char* search_best_order(int page_order);
char* page_alloc(int page_need);
void page_free(int index);
void page_merge(int index);
void show_page();
char* d_alloc(int size);
void memory_reserve(char* start,char* end);
void show_pool_info();
