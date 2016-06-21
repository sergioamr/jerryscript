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
* @brief Simulates the disk access to create a writtable memory section
* to help on the transactions between the UART and the Javascript code
* this is a basic stub, do not expect a full implementation.
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

#include "code_memory.h"
#include "ihex/kk_ihex_read.h"

static struct code_memory memory_code;

CODE *csopen(const char * filename, const char * mode) {
	memory_code.curoff = 0;
	memory_code.curend = 0;
	memory_code.maxsize = MAX_JAVASCRIPT_CODE_LEN;
	return &memory_code;
}

int csseek(CODE *stream, long int offset, int whence) {
	switch (whence) {
	case SEEK_CUR:
		stream->curoff += offset;
		break;
	case SEEK_SET:
		stream->curoff = offset;
		break;
	case SEEK_END:
		stream->curoff = stream->curend + offset;
		break;
	default:
		return (EOF);
	}

	if (stream->curoff < 0)
		stream->curoff = 0;

	if (stream->curoff >= stream->curend)
		stream->curoff = stream->curend;

	return 0;
}

size_t cswrite(const char * ptr, size_t size, size_t count, CODE * stream) {
	unsigned int t;

	size *= count;
	if (size + stream->curoff >= stream->maxsize) {
		size = stream->maxsize - stream->curoff;
	}

	char *pos = &stream->data[stream->curoff];
	for (t = 0; t < size; t++) {
		*(pos++) = *ptr++;
	}

	stream->curoff += size;
	if (stream->curend < stream->curoff)
		stream->curend = stream->curoff;

	return size;
}

size_t csread(char * ptr, size_t size, size_t count, CODE * stream) {
	return (EOF);
}

int csclose(CODE * stream) {
	return (EOF);
}

#ifdef TESTING
void main() {

	CODE *myfile;

	myfile = csopen("test.js", "rw+");
	printf(" Getting memory %p \n", myfile);

	cswrite("01234567890123456789\0", 21, sizeof(char), myfile);
	printf("[%s] %i \n", myfile->data, myfile->curoff);
	csseek(myfile, 10, SEEK_SET);
	cswrite("ABCDEFGHIK\0", 11, sizeof(char), myfile);
	printf("[%s] %i \n", myfile->data, myfile->curoff);

	csseek(myfile, -10, SEEK_END);
	cswrite("012345", 5, sizeof(char), myfile);
	printf("[%s] %i \n", myfile->data, myfile->curoff);

	printf(" End \n");
}
#endif
