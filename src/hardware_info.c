#include "hardware_info.h"
#include "mailbox.h"
#include "uart.h"
#include "utils.h"
/*
GET_FIRMWARE_VERSION: 4 bytes
GET_BOARD_MODEL: 4 bytes
GET_BOARD_REVISION: 4 bytes
GET_MAC_ADDRESS: 6 bytes
GET_TEMPERATURE: 4 bytes
GET_MAX_CLOCK_RATE: 8 bytes
GET_MIN_CLOCK_RATE: 8 bytes
GET_CLOCK_RATE: 8 bytes
SET_CLOCK_RATE: 8 bytes
GET_VOLTAGE: 8 bytes
SET_VOLTAGE: 8 bytes
GET_MAX_VOLTAGE: 8 bytes
GET_MIN_VOLTAGE: 8 bytes
GET_MAX_TEMPERATURE: 4 bytes
GET_MIN_TEMPERATURE: 4 bytes
ALLOCATE_MEMORY: 8 bytes
LOCK_MEMORY: 4 bytes
UNLOCK_MEMORY: 4 bytes
RELEASE_MEMORY: 4 bytes
EXECUTE_CODE: 4 bytes
*/
unsigned int get_buffer_size(unsigned int tag_identifier)
{
  switch (tag_identifier)
  {
  case GET_FIRMWARE_VERSION:
  case GET_BOARD_MODEL:
  case GET_BOARD_REVISION:
  case GET_TEMPERATURE:
  case GET_MAX_TEMPERATURE:
  case GET_MIN_TEMPERATURE:
  case LOCK_MEMORY:
  case UNLOCK_MEMORY:
  case RELEASE_MEMORY:
    return 4;
    break;
  case GET_ARM_MEMORY:
  case GET_MAC_ADDRESS:
  case GET_MAX_CLOCK_RATE:
  case GET_MIN_CLOCK_RATE:
  case SET_CLOCK_RATE:
  case GET_VOLTAGE:
  case SET_VOLTAGE:
  case GET_MAX_VOLTAGE:
  case GET_MIN_VOLTAGE:
  case ALLOCATE_MEMORY:
  case GET_CLOCK_RATE:
    return 8;
    break;
  case GET_CLOCKS:
    return 8 * 4; // 4 core processor
  default:
    return 0;
    break;
  }
}

void get_hw_info(unsigned int tag_identifier, unsigned int *request_message) 
{
  volatile unsigned int  __attribute__((aligned(16))) mailbox[48];
  unsigned int buffer_size = get_buffer_size(tag_identifier);
  if (buffer_size == 0) return;
  mailbox[0] = 6 * 4 + buffer_size; // buffer size in bytes
  mailbox[1] = REQUEST_CODE;
  // tags begin
  mailbox[2] = tag_identifier;
  mailbox[3] = buffer_size; // maximum of request and response value buffer's length.
  mailbox[4] = TAG_REQUEST_CODE;
  unsigned int buffer_addr_len = (buffer_size >> 2);
  // copy request message
  memcpy(mailbox + 5, request_message, buffer_size);
  // tags end
  mailbox[5 + buffer_addr_len] = END_TAG;

  // channel 8 for ARM -> VC
  mailbox_call(mailbox, 8);

  if (mailbox[1] != RESPONSE_CODE_SUCCESS) {
    uart_write_string("message did not be handled appropriately!\n");
    return;
  }
  for (int i = 1; i <= buffer_addr_len; i++) {
    unsigned int value = mailbox[4 + i];
    uart_write_string("0x");
    uart_write_no_hex(value);
    kuart_write(' ');
  }
  uart_write_string("\n");
}

void print_hw_info(void) {
  /* print hardware information */
  unsigned int request_message[8] = {0};
  uart_write_string("BOARD REVISION: ");
  get_hw_info(GET_BOARD_REVISION, &request_message);
  uart_write_string("ARM MEMORY: ");
  get_hw_info(GET_ARM_MEMORY, &request_message);
  uart_write_string("MAC ADDRESS: ");
  get_hw_info(GET_MAC_ADDRESS, &request_message);
  uart_write_string("TEMPERATURE: ");
  get_hw_info(GET_TEMPERATURE, &request_message);
}