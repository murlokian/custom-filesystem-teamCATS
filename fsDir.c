/**************************************************************
* Class::  CSC-415-03 Spring 2026
* Name:: Patricia Ann Payumo
* Student IDs:: 923555591
* GitHub-Name::  milo-pa
* Group-Name::  CATS
* Project:: Basic File System
*
* File:: fsDir.c
*
* Description::  This is the file for the directory related functions.  
*
**************************************************************/

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

#include "fsDir.h"
#include "mfs.h"
#include "fsFreeSpace.h"
#include "fsLow.h"
#include "fsVCB.h"
#include "debug.h"

directory_entry *createDirectory(int initialEntryCount, directory_entry *parent) {

	// Check that initial entry count is at least 2 for "." and ".." entries
	if (initialEntryCount < 2)
	{
		DEBUG("initialEntryCount must be at least 2.");
		return NULL;
	}

	// Get block size from VCB for memory allocation and disk I/O calculations
	int blockSize = VCBGlobal->blockSize;

	// Calculate memory for DE pointer, rounding up to the nearest block size
	// so that it can be passed to LBAread and LBAwrite
	int memoryNeeded = initialEntryCount * sizeof(directory_entry);
	int blocksNeeded = (memoryNeeded + (blockSize - 1)) / blockSize;
	memoryNeeded = blocksNeeded * blockSize;

	// Calculate the number of directory entries that can now fit in the allocated memory
	// since you may be able to fit more due to rounding up to the nearest block size
	int entryCount = memoryNeeded / sizeof(directory_entry);

	DEBUG("Creating directory with %d entries, which requires %d bytes and %d blocks.",
		  initialEntryCount, memoryNeeded, blocksNeeded);

	directory_entry *newDir = calloc(1, memoryNeeded);
	if (newDir == NULL)
	{
		DEBUG_ERR("malloc failed: fsDir.c");
		return NULL;
	}

	// Get blocks on disk for this directory
	int location = getBlocks(blocksNeeded); // getBlocks to be written in fsFreeSpace.c
	if (location < 0)
	{
		free(newDir);
		newDir = NULL;
		DEBUG("getBlocks failed: fsDir.c");
		return NULL;
	}

	// Get timestamp for directory metadata once
	time_t curDate = time(NULL);

	// Populate "." entry which points to the directory itself. 
	// This is required for all directories including root.
	strcpy(newDir[0].name, ".");
	newDir[0].entryType = true;
	newDir[0].size = entryCount * sizeof(directory_entry);
	newDir[0].creationDate = curDate;
	newDir[0].lastEditDate = curDate;
	newDir[0].lastAccessDate = curDate;
	newDir[0].exts[0].location = location;
	newDir[0].exts[0].length = blocksNeeded * blockSize;

	// if parent parameter is NULL, set parent to the new directory
	// ".." should point back to the root itself
	if (parent == NULL)
	{
		parent = newDir;
	}

	// Populate ".." entry which points to the parent directory.
	// This is required for all directories including root
	strcpy(newDir[1].name, "..");
	newDir[1].entryType = true;
	newDir[1].size = parent[0].size;
	newDir[1].creationDate = parent[0].creationDate;
	newDir[1].lastEditDate = parent[0].lastEditDate;
	newDir[1].lastAccessDate = parent[0].lastAccessDate;
	newDir[1].exts[0].location = parent[0].exts[0].location;
	newDir[1].exts[0].length = parent[0].exts[0].length;

	// Mark all unused entries as not used by setting name to "\0"
	for (int i = 2; i < entryCount; i++)
	{
		newDir[i].name[0] = '\0';
	}

	// Write entire directory to disk
	if (LBAwrite(newDir, blocksNeeded, location) != blocksNeeded)
	{
		DEBUG("LBAwrite failed: fsDir.c");
		free(newDir);
		newDir = NULL;
		return NULL;
	}

	return newDir;
}
