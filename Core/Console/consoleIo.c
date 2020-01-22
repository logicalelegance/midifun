// Console IO is a wrapper between the actual in and output and the console code
// In an embedded system, this might interface to a UART driver.

#include "consoleIo.h"
#include "stm32f3xx_hal_uart.h"
#include <stdio.h>

static UART_HandleTypeDef *con_uart;

typedef int getch_status_t;

#define GOT_CHAR 1
#define NO_CHAR_AVAILABLE 0

static getch_status_t getch_noblock(char *c) {
	if (HAL_UART_Receive(con_uart, (uint8_t*) c, 1, 1) == HAL_OK) {

		// Echo
		HAL_UART_Transmit(con_uart, (uint8_t*) c, 1, 1);
		return GOT_CHAR;
	}
	return NO_CHAR_AVAILABLE;
}

eConsoleError ConsoleIoInit(UART_HandleTypeDef *uart) {
	// ASSERT(uart != NULL);
	con_uart = uart;
	return CONSOLE_SUCCESS;
}

eConsoleError ConsoleIoReceive(uint8_t *buffer, const uint32_t bufferLength,
		uint32_t *readLength) {
	uint8_t i = 0;
	char ch;
	getch_status_t status = GOT_CHAR;

	if (getch_noblock(&ch) == GOT_CHAR) {
		HAL_UART_Transmit(con_uart, (uint8_t*) &ch, 1, 11);
		while ((status == GOT_CHAR) && (i < bufferLength)) {
			buffer[i] = (uint8_t) ch;
			i++;
			status = getch_noblock(&ch);
		}
		*readLength = i;
	}
	return CONSOLE_SUCCESS;
}

eConsoleError ConsoleIoSend(const uint8_t *buffer, const uint32_t bufferLength,
		uint32_t *sentLength) {
	printf("%s", (char*) buffer);
	*sentLength = bufferLength;
	return CONSOLE_SUCCESS;
}

eConsoleError ConsoleIoSendString(const char *buffer) {
	printf("%s", buffer);
	return CONSOLE_SUCCESS;
}

