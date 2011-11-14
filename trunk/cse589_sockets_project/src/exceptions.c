/*
 * exceptions.c
 *
 *  Created on: Nov 10, 2011
 *      Author: gassa
 */
#include "../include/exceptions.h"

exception_type_name exception_mapper[] = { {"DEBUG:  "}, {"NOTE:  "}, {"INFO:  "}, {"WARNING:  "}, {"ERROR:  "}, {"FATAL_ERROE:  "} };
void throw_exception(const int exception_type, const char *formatted_msg, ...) {
	va_list args;
	va_start(args, formatted_msg);

	if( exception_type >= ERROR ) {
		fprintf(EXCEPTION_OUTPUT, exception_mapper[exception_type].name);
		vfprintf(EXCEPTION_OUTPUT, formatted_msg, args);
		fputc('\n', EXCEPTION_OUTPUT);
	}else{
		fprintf(STANDAR_OUTPUT, exception_mapper[exception_type].name);
		vfprintf(STANDAR_OUTPUT, formatted_msg, args);
		fputc('\n', STANDAR_OUTPUT);
	}

	va_end(args);
	exit(1);
}

