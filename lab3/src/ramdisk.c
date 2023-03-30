#include "mini_uart.h"

typedef struct
{
	char magic[6];		//header "070701"	
	char ino[8];		//i-node number
	char mode[8];		//permission
	char uid[8];		//user ID
	char gid[8];		//group ID
	char nlink[8];		//number of hard link
	char mtime[8];		//modification time
	char filesize[8];	//file size
	char devmajor[8];	//device major name
	char devminor[8];	//device minor name
	char rdevmajor[8];	//remote device major name
	char rdevminor[8];	//remote device minor name
	char namesize[8];	//length of filename in bytes
	char check[8];		//always set to zero by writer and ignored by reader
}cpio;					//hardlinked files are handled by setting the file size to zero for each entry except for first one

int bufcmp(void *str1,void *str2,int n)
{
	unsigned char *a = str1;
	unsigned char *b = str2;
	while(n--)
	{
		if(*a != *b)
		{
			return 1;
		}
		a++;
		b++;
	}
	return 0;
}

static char* file_name[30];
static char* file_content[30];
static int	ns_arr[30];
static int	fs_arr[30];
static int	index;

void init_rd(char *buffer)
{
	index = 0;
	uart_send_string("Offset\tSize\tUID.GID\t\tFile_name\r\n");
	while(!bufcmp(buffer,"070701",6) && bufcmp(buffer+sizeof(cpio),"TRAILER!!!",10))		//start by "070701" , and not yet end by "TRAILER!!!"
	{
		cpio *header = (cpio*)buffer;
		int ns = hex_to_int(header->namesize,8);
		int ns_offset = (4-((ns+sizeof(cpio))%4)+4)%4;
		int fs = hex_to_int(header->filesize,8);
		int fs_offset = (4-(fs%4)+4)%4;
		uart_int((unsigned int)(sizeof(cpio)+ns+ns_offset));	//offset(header + full path name + padding)
		uart_send('\t');
		uart_int(fs);								//file size
		uart_send('\t');
		uart_int(hex_to_int(header->uid,8));		//UID
		uart_send('.');
		uart_int(hex_to_int(header->gid,8));		//GID
		uart_send('\t');
		uart_send_string(buffer+sizeof(cpio));		//file name
		uart_send_string("\r\n");
		file_name[index] = buffer+sizeof(cpio);
		file_content[index] = buffer+sizeof(cpio)+ns+ns_offset;
		ns_arr[index] = ns;
		fs_arr[index] = fs;
		index++;
		buffer += (sizeof(cpio)+ns+ns_offset+fs+fs_offset);		//jump to next file
	}
	return;
}

void ls()
{
	for(int i=0;i<index;i++)
	{
		uart_send_num_string(file_name[i],ns_arr[i]);
		uart_send_string("\r\n");
	}
	return;
}

void cat()
{
	for(int i=0;i<index;i++)
	{
		uart_send_string("Filename: ");	
		uart_send_num_string(file_name[i],ns_arr[i]);
		uart_send_string("\r\n");
		uart_send_num_string(file_content[i],fs_arr[i]);
		uart_send_string("\r\n");
	}
	return;
}

char* find_prog(char* buffer,char* target)
{
	while(!bufcmp(buffer,"070701",6) && bufcmp(buffer+sizeof(cpio),"TRAILER!!!",10))		//start by "070701" , and not yet end by "TRAILER!!!"
	{
		cpio *header = (cpio*)buffer;
		int ns = hex_to_int(header->namesize,8);
		int ns_offset = (4-((ns+sizeof(cpio))%4)+4)%4;
		int fs = hex_to_int(header->filesize,8);
		int fs_offset = (4-(fs%4)+4)%4;
		char prog_name[100];
		int i;
		for(i=0;i<ns;i++)
		{
			prog_name[i] = *(buffer+sizeof(cpio)+i);		//store program name
		}
		prog_name[i] = '\0';
		if(!bufcmp(&prog_name,target,ns))					//compare target program name
		{
			uart_send_string("find program !!!\r\n");
			return buffer+sizeof(cpio)+ns+ns_offset;			//return file's content
		}
		buffer += (sizeof(cpio)+ns+ns_offset+fs+fs_offset);		//jump to next file
	}
	return 0;
}
