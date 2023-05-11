
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
	int pad = (int)alloc_base%8;
	pad = (8-(pad%8)+8)%8;
	alloc_base += pad;
	char* alloc = alloc_base;

	pad = size%8;
	pad = (8-(pad%8)+8)%8;
	alloc_base += size;
	return alloc;
}

int log_2(int num)
{
	int res = 0;
	while(num/2)
	{
		res++;
		num /= 2;
	}
	return res;
}

int pow_2(int num)
{
	int res = 1;
	for(int i=0;i<num;i++)
	{
		res *= 2;
	}
	return res;
}
