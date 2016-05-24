/* Copyright 2016 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
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

#include <zephyr.h>
#include <misc/printk.h>
#include <misc/shell.h>

#include "jerry.h"

#if defined(CONFIG_STDOUT_CONSOLE)
#include <stdio.h>
#define PRINT           printf
#else
#include <misc/printk.h>
#define PRINT           printk
#endif

/**
 * Jerryscript simple test loop
 */
int jerryscript_test ()
{
    jerry_completion_code_t ret_code;
    const char script [] = "print('= Hi Js!='); for (t=0; t<10; t++) print ('t='+(t+1));";
    ret_code = jerry_run_simple ((jerry_api_char_t *) script, strlen(script), JERRY_FLAG_EMPTY);
    return ret_code;
}

static int
shell_cmd_syntax_help(int argc, char* argv[])
{
    printf("version jerryscript & zephyr versions\n");
    return 0;
}

static int
shell_cmd_version(int argc, char* argv[])
{
    uint32_t version = sys_kernel_version_get();

    printf("Jerryscript %s %s %s\n", jerry_branch_name, jerry_build_date, jerry_commit_hash);
    printk("Zephyr version %d.%d.%d\n",
            SYS_KERNEL_VER_MAJOR(version),
            SYS_KERNEL_VER_MINOR(version),
            SYS_KERNEL_VER_PATCHLEVEL(version));
    return 0;
}

static int
shell_cmd_test(int argc, char* argv[])
{
    return jerryscript_test();
}

static int
shell_cmd_handler(int argc, char* argv[])
{
    printf("START %d\n", argc);
    for (int t=0; t<argc; t++) {
        printf("%d[%s]\n",t, argv[t]);
    }

    if (argc<1)
        return -1;

    printf("END\n");
    return 0;
}

const struct shell_cmd commands[] = { { "syntax", shell_cmd_syntax_help },
                                      { "version", shell_cmd_version },
                                      { "test", shell_cmd_test },
                                      { NULL, NULL } };

void main(void)
{
    printf("Jerry Compilation " __DATE__ " " __TIME__ "\n");
    jerry_flag_t flags = JERRY_FLAG_EMPTY;
    jerry_init (flags);
    shell_register_app_cmd_handler(shell_cmd_handler);
    shell_init("js> ", commands);
}

void __attribute__ ((noreturn)) abort()
{
    while(1) { };
}

void __attribute__ ((noreturn)) exit(int value)
{
    while(1) { };
}
