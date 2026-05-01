/**************************************************************
* Class::  CSC-415-03 Spring 2026
* Name:: Milo Pesce Ares, Allie Young
* Student IDs:: 921926821, 923129607
* GitHub-Name:: milo-pa
* Group-Name:: CATS
* Project:: Basic File System
*
* File:: debug.h
*
* Description:: Debugging macros for file system assignment.
*
* This file contains debugging macros for use throughout the file 
* system implementation. If the macro is not defined, the debug 
* statements will compile away entirely.
*
**************************************************************/

#ifndef DEBUG_H
#define DEBUG_H

#ifdef DEBUG_ENABLED

	#include <stdio.h>
	#include <string.h>
	#include <errno.h>

	// Debug print function similar to fprintf() writing to stderr but includes 
	// function and file line number information
	#define DEBUG(fmt, ...) \
		fprintf(stderr, "[DEBUG] %s():%d - " fmt "\n", __func__, __LINE__, ##__VA_ARGS__)

	// Debug print function similar to perrror() but includes function and file 
	// line number information
	#define DEBUG_ERR(fmt, ...) \
		fprintf(stderr, "[ERROR] %s():%d - " fmt ": %s\n", __func__, __LINE__, ##__VA_ARGS__, \
		strerror(errno))

#else

	// These compile away entirely

	// Debug print function similar to fprintf() writing to stderr but includes 
	// function and file line number information
	#define DEBUG(...) ((void)0)

	// Debug print function similar to perrror() but includes function and file 
	// line number information
	#define DEBUG_ERR(...) ((void)0) 

#endif

// Helper to format times into human readable format for debug printing  
char* formatTime(time_t t, char* buf, size_t bufSize);

#endif