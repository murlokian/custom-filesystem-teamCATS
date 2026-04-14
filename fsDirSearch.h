/**************************************************************
* Class::  CSC-415-03 Spring 2026
* Name::  Milo Pesce Ares
* Student IDs::  921926821
* GitHub-Name::  milo-pa
* Group-Name::  CATS
* Project:: Basic File System
*
* File:: fsDirSearch.h
*
* Description:: This is a header file for helper functions
* related to searching for directory entries and parsing paths.
*
**************************************************************/

#ifndef FSDIRSEARCH_H
#define FSDIRSEARCH_H

#include "fsDir.h"

typedef int dir_index;

// Struct to hold information about the result of parsing a path. This is used to return
// multiple values from the parsePath function.
typedef struct parsePathInfo {
	directory_entry *parentDir;
	dir_index entryIndex;
	char * lastElementName;
} parsePathInfo;

// Parses the given path and passes back information about the path in the 
// parsePathInfo struct. 
// Returns 0 on success, -1 on failure
int parsePath(const char * path, parsePathInfo * ppi);

// Finds the index of the directory entry with the given name in the given directory.
// Returns the index of the entry if found, -1 if not found.
dir_index findDirEntry(const directory_entry * dir, const char * name);

// Finds the index of the first unused directory entry in the given directory.
// Returns the index of the first unused entry if found, -1 if no unused entries.
dir_index findUnusedDirEntry(const directory_entry * dir);

#endif
