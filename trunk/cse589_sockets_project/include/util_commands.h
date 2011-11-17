/*
 * util_commands.c
 *
 *  Created on: Nov 10, 2011
 *      Author: gassa
 */

#ifndef UTIL_COMMANDS_C_
#define UTIL_COMMANDS_C_

#include "util_header.h"
#include "util_network.h"

#define EMPTY_COMMAND    -1

typedef struct {
	char* name; /* command name: "info", "connect", ... */
} command_table;

void check_command(int argc, char** argv);
void info_handler();
void ready();
void establish_connection(char* argv);
void show_connections();
void self_token_handler();
void all_tokens_handler();
void exit_handler();
void cmd_not_found();
void run_cmd(char line[]);

#endif /* UTIL_COMMANDS_C_ */
