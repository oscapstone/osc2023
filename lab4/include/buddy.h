void init_buddy();
char* search_best_order(int page_order);
char* page_alloc(int page_need);
void page_free(int index);
void page_merge(int index);
void show_page();
