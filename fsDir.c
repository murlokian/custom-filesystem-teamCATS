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

directory_entry* rootGlobal = NULL;
directory_entry* cwdGlobal = NULL;

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

directory_entry * loadDir(directory_entry * dir) {
	// If passed NULL, load root directory  
	if(dir == NULL) {
		directory_entry* deez = malloc(VCBGlobal->blockSize);
		if(deez == NULL) {
			DEBUG("malloc failed");
			return NULL;
		}

		// Read first block to get root's "." entry  
		if(LBAread(deez, 1, VCBGlobal->rootLocation) != 1) {
			DEBUG("lbaread failed loading 1st root block");
			free(deez);
			deez = NULL;
			return NULL;
		}

		directory_entry* root = loadDir(&deez[0]);
		free(deez);
		return root;
	}

	// Exit if entry is file  
	if(dir->entryType == 0) {
		DEBUG("loadDir called on file");
		return NULL;
	}

	// Calculate size of all entries  
	int numBlocks = 0;
	for(int i = 0; i < NUM_DIR_EXTENTS; i++) {
		if(dir->exts[i].location == 0) {
			break;
		}
		numBlocks += dir->exts[i].length;
	}

	// Allocate memory for entry array  
	directory_entry* deez = malloc(numBlocks * VCBGlobal->blockSize);
	if(deez == NULL) {
		DEBUG("malloc failed");
		return NULL;
	}

	// vvv TESTING CODE vvv
	// Using first extent only, so we can test our mfs functions before getNextExtent is
	// written. This should be fine since our directories are contiguous  
	if(dir->exts[0].location == 0 || dir->exts[0].length == 0) {
		DEBUG("directory first extent invalid");
		free(deez);
		deez = NULL;
		return NULL;
	}

	if(LBAread(deez, dir->exts[0].length, dir->exts[0].location) != dir->exts[0].length) {
		DEBUG("lbaread failed");
		free(deez);
		deez = NULL;
		return NULL;
	}
	// ^^^ TESTING CODE ^^^

	// // Iterate through extents  
	// int offset = 0;
	// int extIndex = 0;
	// extent* ext;
	// while(true) {
	// 	ext = getNextExtent(dir, &extIndex);

	// 	// No more extents  
	// 	if(ext == NULL) {
	// 		break;
	// 	}

	// 	// Load into in memory array  
	// 	if(LBAread((char*)deez + offset, ext->length, ext->location) != ext->length) {
	// 		DEBUG("lbaread failed");
	// 		free(deez);
	// 		deez = NULL;
	// 		return NULL;
	// 	}
	// 	offset += ext->length * VCBGlobal->blockSize;
	// }

	return deez;
}