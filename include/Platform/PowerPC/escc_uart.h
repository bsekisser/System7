#ifndef ESCC_UART_H
#define ESCC_UART_H

#include <stdbool.h>

/* Initialize ESCC for 115200 baud, 8N1 */
void escc_init(void);

/* Send a character */
void escc_putchar(char c);

/* Send a string */
void escc_puts(const char *str);

/* Check if character is available */
bool escc_rx_ready(void);

/* Receive a character (blocking) */
char escc_getchar(void);

#endif /* ESCC_UART_H */
