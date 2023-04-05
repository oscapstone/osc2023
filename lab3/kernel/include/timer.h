typedef struct{
	unsigned char used;
	int remain;
	int (*fn)(void*);
	void* arg;
} timer_Q;

int add_timer(int (*fn)(void*), int after, void* arg);
int set_timeout(char* message, int second);
int timer_walk(int sec);

int init_timer_Q(void);