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

#ifndef __CODE_MEMORY_H__
#define __CODE_MEMORY_H__

#define MAX_JAVASCRIPT_CODE_LEN 4096

struct code_memory {
	char data[MAX_JAVASCRIPT_CODE_LEN];
	size_t curoff;
	size_t curend;
	size_t maxsize;
};

#define CODE struct code_memory

CODE *csopen(const char *filename, const char *mode);
int csseek(CODE *stream, long int offset, int whence);
size_t cswrite(const char *ptr, size_t size, size_t count, CODE *stream);
size_t csread(char *ptr, size_t size, size_t count, CODE *stream);
int csclose(CODE * stream);

#endif
