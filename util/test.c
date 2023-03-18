#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned long long chk(char *content, unsigned sz) {
	unsigned long long res = 0;
	for (int i = 0; i < sz; i += 8) {
		res += *(unsigned long long *)(content + i);
	}
	unsigned remains = (sz % 8);
	if (remains != 0) {
		res += (*(unsigned long long *)(content + sz - 8) & (~(1 << remains))) << (8 - remains);
	}
	return res;
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("Usage: %s <file path>", *argv);
		return 1;
	}
	FILE *ptr;
	char c;
	ptr = fopen(argv[1], "rb");
	unsigned sz;
	fseek(ptr, 0L, SEEK_END);
	sz = ftell(ptr);
	fseek(ptr, 0L, SEEK_SET);
	char *content = malloc(sz);
	for (int i = 0; i < sz; i++) {
		content[i] = fgetc(ptr);
	}
	printf("checksum: %llX\n", chk(content, sz));
	return 0;
}
