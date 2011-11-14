/*
 * exceptions.c
 *
 *  Created on: Nov 10, 2011
 *      Author: gassa
 */
#ifndef _EXCEPTIONS_H
#define _EXCEPTIONS_H

#include <stdarg.h>
#include "util_header.h"

#define STANDAR_OUTPUT stdout
#define EXCEPTION_OUTPUT  stderr

typedef struct {
	char* name;
} exception_type_name;

void throw_exception(const int exception_type, const char* msg, ...);
#endif /* _EXCEPTIONS_H */
