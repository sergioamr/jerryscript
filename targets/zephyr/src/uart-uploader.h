/* Copyright 2015 Intel Corporation
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

#ifndef __UART_UPLOADER_H__
#define __UART_UPLOADER_H__

// TODO @sergioamr Move this definition to the configuration
#define CONFIG_UART_UPLOAD_HANDLER_STACKSIZE 1024

void code_runner(int arg1, int arg2);
void uart_uploader_init(void);

#endif
