
int strcmp(char* a,char* b)
{
	while(*a)
	{
		if(*a != *b)
		{
			break;
		}
		a++;
		b++;
	}
	return *a-*b;
}

extern unsigned char _end;
char* alloc_base = (char*)&_end;

void* memalloc(int size)
{
	char* alloc = alloc_base;
	alloc_base += size;
	return alloc;
}
