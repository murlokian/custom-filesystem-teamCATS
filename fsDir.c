/**************************************************************
* Class::  CSC-415-03 Spring 2026
* Name:: Patricia Ann Payumo, Allie Young, Rosa Melton, Milo Pesce Ares
* Student IDs:: 923555591, 923129607, 921337778, 921296821
* GitHub-Name::  milo-pa
* Group-Name::  CATS
* Project:: Basic File System
*
* File:: fsDir.c
*
* Description::  Implements fsDir.h functions for core directory
* 	operations
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

// sentinel node to indicate location to secondary extent table = -2 
#define SECONDARY_EXT_SENTINEL ((uint32_t) 0xFFFFFFFE)

// sentinel node to indicate location to tertiary extent table  = -3
#define TERTIARY_EXT_SENTINEL ((uint32_t) 0xFFFFFFFD)

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

	DEBUG("Creating directory with %d entries, %d bytes, and %d blocks",
		  entryCount, memoryNeeded, blocksNeeded);

	directory_entry *newDir = calloc(1, memoryNeeded);
	if (newDir == NULL)
	{
		DEBUG_ERR("malloc failed: fsDir.c");
		return NULL;
	}

	// Get contiguous blocks on disk  
	// NOTE: directory entries are preferably contiguous since they are small, 
	// do not change size, and we are the file system  
	extent* exts = getBlocks(blocksNeeded, blocksNeeded);
	if (exts == NULL)
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
	for(int i = 0; i < NUM_DIR_EXTENTS && exts[i].length != 0; i++) {
		newDir[0].exts[i] = exts[i];
	}

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
	for(int i = 0; i < NUM_DIR_EXTENTS && parent[0].exts[i].length != 0; i++) {
		newDir[1].exts[i] = parent[0].exts[i];
	}

	// Mark all unused entries as not used by setting name to "\0"
	for (int i = 2; i < entryCount; i++)
	{
		newDir[i].name[0] = '\0';
	}

	// Write entire directory to disk
	if (writeDirectory(newDir) != 0)
	{
		free(newDir);
		newDir = NULL;
		DEBUG("writeDirectory failed");
		return NULL;
	}

	// Free used resources  
	free(exts);
	exts = NULL;

	return newDir;
}

int writeDirectory(directory_entry *dir) {

	int blockSize = VCBGlobal->blockSize;
	int blocksNeeded = (dir[0].size + (blockSize - 1)) / blockSize;

	DEBUG("Writing directory with size %lu bytes and %d blocks",
		  dir[0].size, blocksNeeded);

	// Assume that directory is contiguously stored on disk
	if (LBAwrite(dir, blocksNeeded, dir->exts[0].location) != blocksNeeded)
	{
		DEBUG("LBAwrite failed: fsDir.c");
		return -1;
	}

	return 0;
}

directory_entry * loadDir(const directory_entry * dir) {
	DEBUG("Not implemented yet");

	// directory_entry * dirs = malloc(dir->size / sizeof(directory_entry));

	// if (dirs == NULL) {
	// 	DEBUG_ERR("malloc");
	// 	return NULL;
	// }

	return NULL;
}

// TODO - implement and move to helper file
extent * getNextExtent(directory_entry * dir, int * extentIndex) {
	DEBUG("Not implemented yet");

    return NULL;
}

// Expands the given file by the given number of blocks and updates the directory 
// entry accordingly. Will create secondary extent tables as needed if the number 
// of extents exceeds NUM_DIR_EXTENTS.
// Returns 0 on success, -1 on failure
// ***TODO*** MAKE HELPER FUNCTIONS IN SEP FILE (fsExtentHelpers)
int growFile(directory_entry * file, int additionalBlocks) {
	DEBUG("Not implemented yet");

	if (file == NULL) {
		DEBUG("file does not exist");
		return -1;
	}
	if (additionalBlocks < 1) {
		DEBUG("cannot add less than one block");
		return -1;
	}

	// get the required blocks from free space first, then populate existing slots or new table
	// if you want 2 extents... why not just alloc + append one extent of N blocks ?
	extent *newExts = getBlocks(additionalBlocks, 1);
	if (newExts == NULL) {
		DEBUG("getBlocks() failed");
		return -1;
	}
	
	// iterate over the current extent array to find the following:

	int firstFree = -1; // look for the first free/unused ext slot used in case 1 or 2

	// look for the sentinel value -2 in length so we know to call either case 2 or 3
	int hasSecondary = 0;
	uint32_t secondaryTableLoc;

	for (int i = 0; i < NUM_DIR_EXTENTS; i++) {
		if (file->exts[i].location == 0 && file->exts[i].length == 0) {
			firstFree = i;
		}
		if (file->exts[i].length == SECONDARY_EXT_SENTINEL) {
			hasSecondary = 1;
			secondaryTableLoc = file->exts[i].location; 
		}
	}

	// 
	int openSlots = NUM_DIR_EXTENTS - firstFree; 


	// iterate over exts array first to find above
	// if we have unused slots, fill those with as many of new exts that we can
	// if overflow, create secondary ext (NEEDS A CHECK)
	// call AddSecondaryExtTable(additionalBlocks) 
	//	  --> calls getBlocks(additionalBlocks, 1)

	

	// THREE CASES TO HANDLE
	// 1) enough free slots to use w/o creating another table
	// 2) can fill to max slots in primary, create secondary
	// 3) add straight to secondary (secondary handles creation of tertiary?)
	
	// HAVE HELPER FUNCTIONS FOR HANDLING EACH 
	// ex) check for when contiguous extents can be merged THEN create tables 
	// ***ADAPTIVE DEFAULT** for number of blocks to add based on the average num
	// of blocks in the last N files in current directory 
	
	// consider adding a trim() for excess unused blocks that would be allocated above
	// this would be a "partial free" ; does freeBlocks() NEED an array of extents?
	// consider changing freeBlocks to take in starting extent and num blocks to loop over 
	// number extents to cover these blocks 

	return -1;
}