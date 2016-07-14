/* Copyright 2016 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <device.h>
#include <uart.h>

#include <zephyr.h>
#include <misc/printk.h>
#include <misc/shell.h>

#include "jerry-api.h"

#if defined (CONFIG_STDOUT_CONSOLE)
#include <stdio.h>
#define PRINT       printf
#else
#include <misc/printk.h>
#define PRINT       printk
#endif

static char *source_buffer = NULL;
static unsigned char flags = 0;

#define VERBOSE 0x01

/**
 * Jerryscript simple test loop
 */
int jerryscript_test ()
{
  jerry_value_t ret_val;

  const char script[] =
  "var test=0; " \
  "for (var t=100; t<1000; t++) test+=t; " \
  "print ('Hi JS World! '+test);";

  printf ("Script [%s]\n", script);
  ret_val = jerry_eval ((jerry_char_t *) script,
      strlen (script),
      false);

  return jerry_value_has_error_flag (ret_val) ? -1 : 0;
} /* jerryscript_test */


static int shell_cmd_verbose (int argc, char *argv[])
{
  printf ("Enable verbose \n");
  flags |= VERBOSE;
  return 0;
} /* shell_cmd_verbose */


static int shell_cmd_syntax_help (int argc, char *argv[])
{
  printf ("version jerryscript & zephyr versions\n");
  return 0;
} /* shell_cmd_syntax_help */


static int shell_cmd_version (int argc, char *argv[])
{
  uint32_t version = sys_kernel_version_get ();

  printf ("Jerryscript API %d.%d\n", JERRY_API_MAJOR_VERSION, JERRY_API_MINOR_VERSION);

  printk ("Zephyr version %d.%d.%d\n", SYS_KERNEL_VER_MAJOR (version),
    SYS_KERNEL_VER_MINOR (version),
    SYS_KERNEL_VER_PATCHLEVEL (version));
  return 0;
} /* shell_cmd_version */


static int shell_cmd_test (int argc, char *argv[])
{
  return jerryscript_test ();
} /* shell_cmd_test */


static int shell_cmd_handler (int argc, char *argv[])
{
  if (argc <= 0)
  {
    return -1;
  }

  unsigned int size = 0;
  for (int t = 0; t < argc; t++)
  {
    size += strlen (argv[t]) + 1;
  }

  source_buffer = (char *) malloc (size);

  char *d = source_buffer;
  unsigned int len;

  for (int t = 0; t < argc; t++)
  {
    len = strlen (argv[t]);
    memcpy (d, argv[t], len);
    d += len;
    *d = ' ';
    d++;
  }

  * (d - 1) = '\0';

  if (flags & VERBOSE)
  {
    printf ("[%s] %lu\n", source_buffer, strlen (source_buffer));
  }

  jerry_value_t ret_val;

  ret_val = jerry_eval ((jerry_char_t *) source_buffer,
    strlen (source_buffer),
    false);

  free (source_buffer);

  if (jerry_value_has_error_flag (ret_val))
  {
    printf ("Failed to run JS\n");
  }

  jerry_release_value (ret_val);

  return 0;
} /* shell_cmd_handler */

#define SHELL_COMMAND(name,cmd) { name, cmd }

const struct shell_cmd commands[] =
{
  SHELL_COMMAND ("syntax", shell_cmd_syntax_help),
  SHELL_COMMAND ("version", shell_cmd_version),
  SHELL_COMMAND ("test", shell_cmd_test),
  SHELL_COMMAND ("verbose", shell_cmd_verbose),
  SHELL_COMMAND (NULL, NULL)
};

static volatile bool data_transmitted;
static volatile bool data_arrived;
static	char data_buf[64];

static void interrupt_handler(struct device *dev)
{
	uart_irq_update(dev);

	if (uart_irq_tx_ready(dev)) {
		data_transmitted = true;
	}

	if (uart_irq_rx_ready(dev)) {
		data_arrived = true;
	}
}

static void write_data(struct device *dev, const char *buf, int len)
{
	uart_irq_tx_enable(dev);

	data_transmitted = false;
	uart_fifo_fill(dev, buf, len);
	while (data_transmitted == false)
		;

	uart_irq_tx_disable(dev);
}

static void read_and_echo_data(struct device *dev, int *bytes_read)
{
	while (data_arrived == false)
		;

	data_arrived = false;

	/* Read all data and echo it back */
	while ((*bytes_read = uart_fifo_read(dev,
		data_buf, sizeof(data_buf)))) {
		write_data(dev, data_buf, *bytes_read);
	}
}

static const char *banner1 = "Send characters to the UART device\r\n";
static const char *banner2 = "Characters read:\r\n";

void main (void)
{
  printf ("Jerry Compilation " __DATE__ " " __TIME__ "\n");
  jerry_init (JERRY_INIT_EMPTY);
  shell_register_app_cmd_handler (shell_cmd_handler);
  shell_init ("js> ", commands);

  struct device *dev;
  uint32_t baudrate, bytes_read, dtr = 0;
  int ret;

  dev = device_get_binding(CONFIG_CDC_ACM_PORT_NAME);
  if (!dev) {
	  PRINT("CDC ACM device not found\n");
	  return;
  }

  PRINT("Wait for DTR\n");
  while (1) {
	  uart_line_ctrl_get(dev, LINE_CTRL_DTR, &dtr);
	  if (dtr)
		  break;
  }
  PRINT("DTR set, start test\n");

  /* They are optional, we use them to test the interrupt endpoint */
  ret = uart_line_ctrl_set(dev, LINE_CTRL_DCD, 1);
  if (ret)
	  PRINT("Failed to set DCD, ret code %d\n", ret);

  ret = uart_line_ctrl_set(dev, LINE_CTRL_DSR, 1);
  if (ret)
	  PRINT("Failed to set DSR, ret code %d\n", ret);

  /* Wait 1 sec for the host to do all settings */
  sys_thread_busy_wait(1000000);

  ret = uart_line_ctrl_get(dev, LINE_CTRL_BAUD_RATE, &baudrate);
  if (ret)
	  PRINT("Failed to get baudrate, ret code %d\n", ret);
  else
	  PRINT("Baudrate detected: %d\n", baudrate);

  uart_irq_callback_set(dev, interrupt_handler);
  write_data(dev, banner1, strlen(banner1));
  write_data(dev, banner2, strlen(banner2));

  /* Enable rx interrupts */
  uart_irq_rx_enable(dev);

  /* Echo the received data */
  while (1) {
	  read_and_echo_data(dev, &bytes_read);
  }

  /* Don't call jerry_cleanup() here, as shell_init() returns after setting
     up background task to process shell input, and that task calls
     shell_cmd_handler(), etc. as callbacks. This processing happens in
     the infinite loop, so JerryScript doesn't need to be de-initialized. */
} /* main */

