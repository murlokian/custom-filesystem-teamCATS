/**************************************************************
* Class::  CSC-415-03 Spring 2026
* Name::  Milo Pesce Ares, Allie Young
* Student IDs::  921926821, 923129607
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
#include <stdlib.h>

#include "fsDir.h"
#include "debug.h"

void freePPI(parsePathInfo* ppi) {
	if(ppi == NULL) {
		return;
	}

	if(ppi->parentDir != NULL && ppi->parentDir != rootGlobal && ppi->parentDir != cwdGlobal) {
		free(ppi->parentDir);
	}
	ppi->parentDir = NULL;

	ppi->entryIndex = -1;

	if(ppi->lastElementName != NULL) {
		free(ppi->lastElementName);
		ppi->lastElementName = NULL;
	}
}

int parsePath(const char* path, parsePathInfo* ppi) {
	directory_entry* parent;
	directory_entry* startDir;
	char* token1;
	char* token2;
	char* saveptr = NULL;
	char* pathCopy;

	if(path == NULL || ppi == NULL) {
		DEBUG("No path or ppi given");
		return -1;
	}

	// Ensure no garbage in ppi  
	ppi->parentDir = NULL;
	ppi->entryIndex = -1;
	ppi->lastElementName = NULL;

	// Path is just "/"  
	if(strcmp(path, "/") == 0) {
		ppi->parentDir = rootGlobal;
		ppi->entryIndex = -2;
		ppi->lastElementName = NULL;
		return 0;
	}

	// Check for absolute or relative path  
	if(path[0] == '/') {
		startDir = rootGlobal;
	} else {
		startDir = cwdGlobal;
	}
	parent = startDir;

	// Get first entry in path  
	pathCopy = strdup(path);
	if(pathCopy == NULL) {
		DEBUG("strdup failed");
		return -1;
	}
	token1 = strtok_r(pathCopy, "/", &saveptr);
	if(token1 == NULL) {
		DEBUG("strtok failed");
		free(pathCopy);
		pathCopy = NULL;
		return -1;
	}

	// Iterate through entries in path  
	while(token1 != NULL) {
		token2 = strtok_r(NULL, "/", &saveptr);
		int index = findDirEntry(parent, token1);

		// No more entries  
		if(token2 == NULL) {
			char* lastElemName = strdup(token1);
			if(lastElemName == NULL) {
				DEBUG("strdup failed");
				if(parent != startDir) {
					free(parent);
					parent = NULL;
				}
				free(pathCopy);
				pathCopy = NULL;
				return -1;
			}

			// This will be -1 if last element does not exist, in cases like mkdir  
			ppi->entryIndex = index;
			ppi->parentDir = parent;
			ppi->lastElementName = lastElemName;

			free(pathCopy);
			pathCopy = NULL;
			return 0;
		}

		// No matching entry found, invalid path  
		if(index == -1) {
			DEBUG("Path has nonexistent entry");
			if(parent != startDir) {
				free(parent);
				parent = NULL;
			}
			free(pathCopy);
			pathCopy = NULL;
			return -1;
		}

		// Path has a file in the middle  
		if(parent[index].entryType == 0) {
			DEBUG("Path has file in the middle");
			if(parent != startDir) {
				free(parent);
				parent = NULL;
			}
			free(pathCopy);
			pathCopy = NULL;
			return -1;
		}

		// Move pointers ahead  
		directory_entry* temp = loadDir(&(parent[index]));
		if(temp == NULL) {
			DEBUG("loadDir failed");
			if(parent != startDir) {
				free(parent);
				parent = NULL;
			}
			free(pathCopy);
			pathCopy = NULL;
			return -1;
		}
		if(parent != startDir) {
			free(parent);
		}
		parent = temp;
		token1 = token2; 
	}
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
