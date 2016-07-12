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
#include "jerry-api.h"

#include "code-memory.h"

void javascript_run_code(const char *file_name) {
  /* Initialize engine */
    jerry_init (JERRY_INIT_EMPTY);	

	CODE *code = csopen(file_name, "r");

	if (code == NULL)
		return;

	size_t len = strlen((char *) code);

	/* Setup Global scope code */
	jerry_value_t parsed_code = jerry_parse ((const jerry_char_t *) code, len, false);
  
	if (!jerry_value_has_error_flag (parsed_code))
	{
		/* Execute the parsed source code in the Global scope */
		jerry_value_t ret_value = jerry_run (parsed_code);

		/* Returned value must be freed */
		jerry_release_value (ret_value);
	} else {
		printf("JerryScript: could not parse javascript\n");
		return;
	}

	/* Parsed source code must be freed */
	jerry_release_value (parsed_code);

	/* Cleanup engine */
	jerry_cleanup ();

	csclose(code);
}

void javascript_run_snapshot(const char *file_name) {

}