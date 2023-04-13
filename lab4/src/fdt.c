#include <stdint.h>
#include "fdt.h"
#include "mini_uart.h"
#include "string.h"

uint32_t to_little_endian_32(uint32_t data)
{
	uint32_t tmp;
	tmp = ((data & 0xFF000000) >> 24) |
		  ((data & 0x00FF0000) >> 8)  |
		  ((data & 0x0000FF00) << 8)  |
		  ((data & 0x000000FF) << 24);
	return tmp;	
}

uint64_t to_little_endian_64(uint64_t data)
{
	uint64_t tmp;
	tmp = ((data & 0xFF00000000000000) >> 56)  |
		  ((data & 0x00FF000000000000) >> 40)  |
		  ((data & 0x0000FF0000000000) >> 24)  |
		  ((data & 0x000000FF00000000) >> 8 )  |
		  ((data & 0x00000000FF000000) << 8 )  |
		  ((data & 0x0000000000FF0000) << 24)  |
		  ((data & 0x000000000000FF00) << 40)  |
		  ((data & 0x00000000000000FF) << 56 );
	return tmp;	
}

void to_little_endian_header(fdt_header* fdt)
{
	if(fdt->magic == 0xD00DFEED)	//check if is little endian
	{
		return;
	}
	fdt->magic = to_little_endian_32(fdt->magic);
	fdt->totalsize = to_little_endian_32(fdt->totalsize);
	fdt->off_dt_struct = to_little_endian_32(fdt->off_dt_struct);
	fdt->off_dt_strings = to_little_endian_32(fdt->off_dt_strings);
	fdt->off_mem_rsvmap = to_little_endian_32(fdt->off_mem_rsvmap);
	fdt->version = to_little_endian_32(fdt->version);
	fdt->last_comp_version = to_little_endian_32(fdt->last_comp_version);
	fdt->boot_cpuid_phys = to_little_endian_32(fdt->boot_cpuid_phys);
	fdt->size_dt_strings = to_little_endian_32(fdt->size_dt_strings);
	fdt->size_dt_struct = to_little_endian_32(fdt->size_dt_struct);
	return;
}

void fdt_header_info(fdt_header* fdt)
{
	uart_send_string("magic : ");
	uart_hex(fdt->magic);
	uart_send_string("\r\n");
	uart_send_string("totalsize : ");
	uart_hex(fdt->totalsize);
	uart_send_string("\r\n");
	uart_send_string("off_dt_struct : ");
	uart_hex(fdt->off_dt_struct);
	uart_send_string("\r\n");
	uart_send_string("off_dt_strings : ");
	uart_hex(fdt->off_dt_strings);
	uart_send_string("\r\n");
	uart_send_string("off_mem_rsvmap : ");
	uart_hex(fdt->off_mem_rsvmap);
	uart_send_string("\r\n");
	uart_send_string("version : ");
	uart_hex(fdt->version);
	uart_send_string("\r\n");
	uart_send_string("last_comp_version : ");
	uart_hex(fdt->last_comp_version);
	uart_send_string("\r\n");
	uart_send_string("boot_cpuid_phys : ");
	uart_hex(fdt->boot_cpuid_phys);
	uart_send_string("\r\n");
	uart_send_string("size_dt_strings : ");
	uart_hex(fdt->size_dt_strings);
	uart_send_string("\r\n");
	uart_send_string("size_dt_struct : ");
	uart_hex(fdt->size_dt_struct);
	uart_send_string("\r\n");
	return;
}

void memresv_block_info(char* fdt,int offset)	//address & size are all 64bit , need 8byte aligned , end both address & size are 0
{
	char* now = fdt+offset;
	uart_send_string("\r\nmemory reservation block start\r\n");
	while(1)
	{
		fdt_reserve_entry *tmp = (fdt_reserve_entry*)now;
		fdt_reserve_entry reserve;
		reserve.address = to_little_endian_64(tmp->address);
		reserve.size = to_little_endian_64(tmp->size);
		now += 16;	//eat struct fdt_reserve_entry
		if(reserve.address == 0x0 && reserve.size == 0x0)
		{
			uart_send_string("\r\nmemory reservation block finish\r\n");
			return;
		}
		uart_send_string("reserved physical address : ");
		uart_hex_64(reserve.address);
		uart_send_string(" , reserved size : ");
		uart_int(reserve.size);
		uart_send_string("\r\n");
	}
	return;
}

void struct_block_info(char* fdt,int struct_offset,int string_offset)	//all token need 4byte aligned , padding with 0x0
{
	char* now = fdt+struct_offset;
	while(1)
	{
		uint32_t token = to_little_endian_32(*((uint32_t*)now));
		int padding;
		switch(token)
		{
			case FDT_BEGIN_NODE:				//followed by node's unit name
					uart_send_string("Node's unit name : ");
					now += 4;					//eat FDT_BEGIN_NODE
					uart_send_string(now);		//name is stored as a null-terminated string , need follow padding bytes
					uart_send_string("\r\n");
					int count = 0;
					while(*now != '\0')
					{
						now++;					//eat name's charactor
						count++;				//for padding consideration
					}
					now++;						//eat NULL
					count++;
					padding = ((4-(count%4))+4)%4;
					now += padding;
					break;
			case FDT_END_NODE:					//followed no extra data
					uart_send_string("End of the Node\r\n");
					now += 4;					//eat FDT_END_NODE
					break;
			case FDT_PROP:						//followed by struct fdt_prop , and then followed property's value , value need follow padding bytes
					uart_send_string("Begin of the Property\r\n");
					now += 4;					//eat FDT_PROP
					fdt_prop *tmp = (fdt_prop*)now;
					fdt_prop prop;
					prop.len = to_little_endian_32(tmp->len);			//length of the property's value , may be zero(empty property)
					prop.nameoff = to_little_endian_32(tmp->nameoff);	//name's offset in string block
					now += 8;											//eat prop.len & prop.nameoff
					char* prop_name = fdt+string_offset+prop.nameoff;	//(string block + nameoff)
					uart_send_string("Property's name : ");
					uart_send_string(prop_name);						//property's name is stored as a null-terminated string
					uart_send_string(" , Property's value : ");
					uart_send_num_string(now,prop.len);					//property's value is stored as a byte string
					uart_send_string("\r\n");
					now += prop.len;									//eat property's value
					padding = ((4-(prop.len%4))+4)%4;
					now += padding;
					break;
			case FDT_NOP:						//followed immediately by the next token , no extra data
					now += 4;					//eat FDT_NOP
					break;
			case FDT_END:						//only one FDT_END token , last token in the structure block , no extra data
					return;
			default:
					uart_send_string("mismatch!!!\r\n");
					break;
		}
	}
	return;
}

void fdt_info(char* fdt_origin)
{
	fdt_header *fdt = (fdt_header*)fdt_origin;
	to_little_endian_header(fdt);
	fdt_header_info(fdt);
	memresv_block_info((char*)fdt,fdt->off_mem_rsvmap);
	struct_block_info((char*)fdt,fdt->off_dt_struct,fdt->off_dt_strings);
	return;
}

void fdt_api(char* fdt_origin,char*(*func)(char* value),char* keyword)
{
	fdt_header *fdt = (fdt_header*)fdt_origin;
	to_little_endian_header(fdt);
	int struct_offset = fdt->off_dt_struct;
	int string_offset = fdt->off_dt_strings;
	char* now = (char*)fdt;
	now += struct_offset;
	while(1)
	{
		uint32_t token = to_little_endian_32(*((uint32_t*)now));
		int padding;
		switch(token)
		{
			case FDT_BEGIN_NODE:				//followed by node's unit name
					now += 4;					//eat FDT_BEGIN_NODE
					int count = 0;
					while(*now != '\0')
					{
						now++;					//eat name's charactor
						count++;				//for padding consideration
					}
					now++;						//eat NULL
					count++;
					padding = ((4-(count%4))+4)%4;
					now += padding;
					break;
			case FDT_END_NODE:					//followed no extra data
					now += 4;					//eat FDT_END_NODE
					break;
			case FDT_PROP:						//followed by struct fdt_prop , and then followed property's value , value need follow padding bytes
					now += 4;					//eat FDT_PROP
					fdt_prop *tmp = (fdt_prop*)now;
					fdt_prop prop;
					prop.len = to_little_endian_32(tmp->len);			//length of the property's value , may be zero(empty property)
					prop.nameoff = to_little_endian_32(tmp->nameoff);	//name's offset in string block
					now += 8;											//eat prop.len & prop.nameoff
					char* prop_name = (char*)fdt+string_offset+prop.nameoff;	//(string block + nameoff)
					if(!strcmp(prop_name,keyword))	//if find keyword
					{
						func((char*)now);	
					}
					now += prop.len;									//eat property's value
					padding = ((4-(prop.len%4))+4)%4;
					now += padding;
					break;
			case FDT_NOP:						//followed immediately by the next token , no extra data
					now += 4;					//eat FDT_NOP
					break;
			case FDT_END:						//only one FDT_END token , last token in the structure block , no extra data
					return;
			default:
					uart_send_string("mismatch!!!\r\n");
					break;
		}
	}
	return;
}
