/**************************************************************
* Class::  CSC-415-03 Spring 2026
* Name::  Milo Pesce Ares
* Student IDs::  921926821
* GitHub-Name::  milo-pa
* Group-Name::  CATS
* Project:: Basic File System
*
* File:: fsDirSearch.c
*
* Description:: Implements fsDir.h helper functions related to 
* searching for directory entries and parsing paths.
*
**************************************************************/

#include <string.h>

#include "fsDir.h"
#include "debug.h"

int parsePath(const char * path, parsePathInfo * ppi) {
	DEBUG("Not implemented yet");
	return -1;
}

dir_index findDirEntry(directory_entry * dir, const char * name) {

	if (dir == NULL || name == NULL) {
		return -1;
	}

	int numEntries = dir[0].size / sizeof(directory_entry);

	// Search through all directory entries for the given name. If found, return 
	// the index of the entry.
	for (int i = 0; i < numEntries; ++i) {
		if (strcmp(dir[i].name, name) == 0) 
		{
			DEBUG("Found '%s' in '%s' at index %d", name, dir[0].name, i);
			return i;
		}
	}

	// If we reach the end of the directory entries without finding an unused
	// directory entry, expand the directory and return the index of the first 
	// entry in the newly allocated space.
	if (strcmp(name, "\0") == 0) 
	{
		DEBUG("Could not find unused entry in '%s', expanding directory", dir[0].name);
		
		if (growFile(dir, sizeof(directory_entry) * numEntries) != 0)
		{
			DEBUG("growFile failed");
			return -1;
		}

		return numEntries;
	}

	DEBUG("Could not find '%s' in '%s'", name, dir[0].name);
	return -1;
}

dir_index findUnusedDirEntry(directory_entry * dir) {

	// findUnusedDirEntry is a wrapper for findDirEntry that searches for the 
	// first entry with name "\0", which signifies an unused entry.
	return findDirEntry(dir, "\0");
}
