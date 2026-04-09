/**************************************************************
* Class::  CSC-415-03 Spring 2026
* Name:: Milo Pesce Ares
* Student IDs:: 921926821
* GitHub-Name:: milo-pa
* Group-Name:: CATS
* Project:: Basic File System
*
* File:: debug.h
*
* Description:: Debugging macros for file system assignment.
*
* This file contains debugging macros for use throughout the file 
* system implementation
*
**************************************************************/

#ifndef DEBUG_H
#define DEBUG_H

#ifdef DEBUG_ENABLED
  #define DEBUG(fmt, ...) \
    fprintf(stderr, "[DEBUG] %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
  #define DEBUG_ERR(fmt, ...) \
    fprintf(stderr, "[DEBUG] %s:%d: " fmt ": %s\n", __FILE__, __LINE__, ##__VA_ARGS__, strerror(errno))
#else
  // These compile away entirely
  #define DEBUG(...) ((void)0)
  #define DEBUG_ERR(...) ((void)0) 
#endif

#endif