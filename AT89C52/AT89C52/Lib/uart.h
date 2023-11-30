#ifndef _UART_H_
#define _UART_H_




void uart_init();

void uart_write(char c);


void uart_write_string(char *str);


char UART_READY();

// RI = 1 la dang co du lieu
char uart_read();

#endif