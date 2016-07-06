/*
* Copyright (c) Intel Corporation
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
* @brief Simple interface to load and run javascript from our code memory stash
*
* Reads a program from the ROM/RAM
*/

// Zephyr includes
#include <zephyr.h>

#include <string.h>

// JerryScript includes
#include "jerry.h"
#include "jerry-api.h"

#include "code-memory.h"

void javascript_run_code(const char *file_name) {
	jerry_object_t *err_obj_p = NULL;
	jerry_value_t res;

	CODE *code = csopen(file_name, "r");

	if (code == NULL)
		return;

	size_t len = strlen((char *) code);

	if (!jerry_parse((jerry_char_t *) code, len, &err_obj_p)) {
		printf("JerryScript: cannot parse javascript\n");
		return;
	}

	if (jerry_run(&res) != JERRY_COMPLETION_CODE_OK) {
		printf("JerryScript: cannot run javascript\n");
		return;
	}

	csclose(code);
}

void javascript_run_snapshot(const char *file_name) {

}