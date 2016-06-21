/*
* Copyright (c) 2011-2012, 2014-2015 Wind River Systems, Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

/**
* @file
* @brief UART-driven uploader
*
* Reads a program from the uart using Intel HEX format.
*
* Designed to be used from Javascript or a ECMAScript object file.
*
* Hooks into the printk and fputc (for printf) modules. Poll driven.
*/

#include <nanokernel.h>
#include <arch/cpu.h>

#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <ctype.h>

#include <device.h>
#include <init.h>

#include <board.h>
#include <uart.h>
#include <toolchain.h>
#include <sections.h>
#include <atomic.h>
#include <misc/printk.h>
#include "ihex/kk_ihex_read.h"

#define CONFIG_UART_UPLOAD_HANDLER_STACKSIZE 2000

#define STACKSIZE CONFIG_UART_UPLOAD_HANDLER_STACKSIZE

static char __stack fiberStack[STACKSIZE];

/*********************************** UART CAPTURE ****************************************/

static struct ihex_state ihex;

static struct nano_fifo avail_queue;
static struct nano_fifo lines_queue;

static struct device *uart_uploader_dev;

/* Control characters for testing and debugging */
#define ESC                0x1b
#define DEL                0x7f

#define MAX_LINE_LEN 256
struct uart_uploader_input {
	int _unused;
	char line[MAX_LINE_LEN];
};

#define MAX_LINES_QUEUED 3
static struct uart_uploader_input buf[MAX_LINES_QUEUED];

#if defined(CONFIG_PRINTK) || defined(CONFIG_STDOUT_CONSOLE)

void uart_clear(void) {
	int i;

	for (i = 0; i < MAX_LINES_QUEUED; i++) {
		nano_fifo_put(&avail_queue, &buf[i]);
	}

}

/**
* Outputs both line feed and carriage return in the case of a '\n'.
* @param c Character to output
* @return The character passed as input.
*/

static int uploader_out(int c)
{
	uart_poll_out(uart_uploader_dev, (unsigned char)c);
	if ('\n' == c) {
		uart_poll_out(uart_uploader_dev, (unsigned char)'\r');
	}
	return c;
}

#endif

extern void __stdout_hook_install(int(*hook)(int));

static int read_uart(struct device *uart, uint8_t *buf, unsigned int size)
{
	int rx;
	rx = uart_fifo_read(uart, buf, size);
	if (rx < 0) {
		/* Overrun issue. Stop the UART */
		uart_irq_rx_disable(uart);
		return -EIO;
	}
	return rx;
}

static uint8_t cur;

void uart_uploader_isr(struct device *unused)
{
	int flush = 0;
	ARG_UNUSED(unused);

	while (uart_irq_update(uart_uploader_dev) &&
		uart_irq_is_pending(uart_uploader_dev)) {
		static struct uart_uploader_input *cmd;
		uint8_t byte;
		int rx;

		if (!uart_irq_rx_ready(uart_uploader_dev)) {
			continue;
		}

		/* Character(s) have been received */

		rx = read_uart(uart_uploader_dev, &byte, 1);
		if (rx < 0) {
			return;
		}

		if (uart_irq_input_hook(uart_uploader_dev, byte) != 0) {
			/*
			* The input hook indicates that no further processing
			* should be done by this handler.
			*/
			printf("- End\n");
			return;
		}

		if (!cmd) {
			cmd = nano_isr_fifo_get(&avail_queue, TICKS_NONE);
			if (!cmd)
				return;
		}

		/* Handle special control characters */
		if (!isprint(byte)) {
			switch (byte) {
			case DEL:
				cmd->line[cur] = '\0';
				printf("[%s]%d\n", cmd->line, cur);
				break;
			case ESC:
				printf("[Clear]\n");
				uart_clear();
				break;
			case '\r':
				cur = 0;
				printf("[ACK]\n");
				break;
			default:
				break;
			}

			continue;
		}

		/* Flush the data into the buffer if end of line or each 32 bytes */

		cmd->line[cur++] = byte;

		if (byte == '\n' || cur > 32) {
			cmd->line[cur] = '\0';
			nano_isr_fifo_put(&lines_queue, cmd);
			cur = 0;
			cmd = NULL;
		}
	}
}

void uploader_input_init(void) {
	uint8_t c;

	uart_irq_rx_disable(uart_uploader_dev);
	uart_irq_tx_disable(uart_uploader_dev);

	uart_irq_callback_set(uart_uploader_dev, uart_uploader_isr);

	/* Drain the fifo */
	while (uart_irq_rx_ready(uart_uploader_dev)) {
		uart_fifo_read(uart_uploader_dev, &c, 1);
	}
	uart_irq_rx_enable(uart_uploader_dev);
}

void uart_uploader_runner(int arg1, int arg2)
{
	ihex_begin_read(&ihex);
	printf("[Waiting for data]\n");

	while (1) {
		struct uart_uploader_input *cmd;
		cmd = nano_fiber_fifo_get(&lines_queue, TICKS_UNLIMITED);
		//printf("[Read][%s]%n \n", cmd->line, strlen(cmd->line));
		ihex_read_bytes(&ihex, cmd->line, strlen(cmd->line));
		nano_fiber_fifo_put(&avail_queue, cmd);
	}
	ihex_end_read(&ihex);
}

/* Data received from the buffer */
ihex_bool_t ihex_data_read(struct ihex_state *ihex,
	ihex_record_type_t type,
	ihex_bool_t checksum_error) {
	if (type == IHEX_DATA_RECORD) {
		unsigned long address = (unsigned long)IHEX_LINEAR_ADDRESS(ihex);
		printf("%n::%n\n", address, ihex->length);
		//(void)fseek(outfile, address, SEEK_SET);
		//(void)fwrite(ihex->data, ihex->length, 1, outfile);
	}
	else if (type == IHEX_END_OF_FILE_RECORD) {
		//(void)fclose(outfile);
		printf("[End of file]\n");
	}
	return true;
}

/* Setup code uploader */
void uart_uploader_init(void) {

	nano_fifo_init(&lines_queue);
	nano_fifo_init(&avail_queue);
	uart_clear();

	task_fiber_start(fiberStack, STACKSIZE, uart_uploader_runner, 0, 0, 7, 0);

	/* Register uart handler */
	uploader_input_init();

} /* uart_uploader_init */

/**
*
* @brief Initialize one UART as the uploader/debug port
*
* @return 0 if successful, otherwise failed.
*/
static int uart_serial_uploader_init(struct device *arg)
{
	ARG_UNUSED(arg);
	uart_uploader_dev = device_get_binding(CONFIG_UART_CONSOLE_ON_DEV_NAME);

	/* Install printk / stdout hook for UART uploader output */
	__stdout_hook_install(uploader_out);
	return 0;
}

/* UART uploader initializes after the UART device itself */
SYS_INIT(uart_serial_uploader_init,
	SECONDARY,
	CONFIG_UART_CONSOLE_INIT_PRIORITY);