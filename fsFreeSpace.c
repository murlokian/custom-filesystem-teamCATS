/**************************************************************
* Class::  CSC-415-03 Spring 2026
* Name::  Allie Young, Milo Pesce Ares
* Student IDs::  923129607, 921296821
* GitHub-Name::  milo-pa
* Group-Name::  CATS
* Project:: Basic File System
*
* File:: fsFreeSpace.c
*
* Description::  This is the freespace system.  We are using a three-level extent 
* table system to track free space.  Extents will be organized into primary and
* secondary levels which are created as needed as the disk becomes fragmented
* and the number of free extents increases.  
*
**************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "fsFreeSpace.h"
#include "fsLow.h"
#include "fsDir.h"
#include "fsVCB.h"
#include "debug.h"

extent* freespaceExtsGlobal = NULL;
bool freeSpaceInitialized = false;

int initFreeSpace(uint32_t blockCount) {

	int blockSize = VCBGlobal->blockSize;

	// Dynamically calculate the location of free space based on the size of 
	// the VCB and block size
	int vcbNumBlocks = (sizeof(volume_control_block) + blockSize - 1) / blockSize;
	int blockAfterVCB = VCB_BLOCK_LOCATION + vcbNumBlocks;

	DEBUG("Address of freespaceLocation: %d", blockAfterVCB);

	int freespaceLocation = blockAfterVCB;

	// Allocate memory on heap for freespace extents
	int numExtents = (NUM_FREESPACE_BLOCKS * blockSize ) / sizeof(extent);
	int bytesNeeded = numExtents * sizeof(extent);

	// *** Fix: Load until sentinel to save memory ***
	extent* extents = malloc(bytesNeeded);
	if(extents == NULL)
	{
		DEBUG("malloc failed in initFreeSpace");
		return -1;
	}

	// Initialize freespace extents
	int firstFreeBlock = freespaceLocation + NUM_FREESPACE_BLOCKS;

	extents[0].location = firstFreeBlock;
	extents[0].length = blockCount - firstFreeBlock;

	// Couldn't this be optimized with calloc? or memset?
	for(int i = 1; i < numExtents; i++) {
		extents[i].location = 0;
		extents[i].length = 0;
	}

	// Keep runtime extents copy
	freespaceExtsGlobal = extents;

	// Write extents to disk  
	int LBAblocks = LBAwrite(extents, NUM_FREESPACE_BLOCKS, freespaceLocation); 
	if(LBAblocks != NUM_FREESPACE_BLOCKS) 
	{
		DEBUG("LBAwrite failed in initFreeSpace");
		free(freespaceExtsGlobal);
		freespaceExtsGlobal = NULL;
		return -1;
	}

	freeSpaceInitialized = true;

	return freespaceLocation;
}

int loadFreeSpace() {

	// Ensure VCBGlobal is initialized
	if(VCBGlobal == NULL)
	{
		DEBUG("VCBGlobal not initialized in loadFreeSpace");
		return -1;
	}
	
	int blockSize = VCBGlobal->blockSize;

	// Allocate memory on heap for freespace extents
	int numExtents = (NUM_FREESPACE_BLOCKS * blockSize ) / sizeof(extent);

	DEBUG("Calculated numExtents: %d", numExtents);

	int bytesNeeded = numExtents * sizeof(extent);

	extent* extents = malloc(bytesNeeded);

	// Read extents from disks
	int LBAblocks = LBAread(extents, NUM_FREESPACE_BLOCKS, VCBGlobal->freespaceLocation);
	if (LBAblocks != NUM_FREESPACE_BLOCKS)
	{
		DEBUG("LBAread failed in loadFreeSpace");
		return -1;
	}

	// Keep runtime extents copy for freespace system
	freespaceExtsGlobal = extents;
	freeSpaceInitialized = true;

	DEBUG("Free space loaded successfully");
	DEBUG("Free Space extent one: location %d, length %d", 
		freespaceExtsGlobal[0].location, freespaceExtsGlobal[0].length);

	return 0;
}

extent* getBlocks(int numBlocks, int minContiguous) {
	// Ensure freespace system has been initialized  
	if(!freeSpaceInitialized)
	{
		DEBUG("Error: Free space system not initialized");
		return NULL;
	}

	int blockSize = VCBGlobal->blockSize;
	int blocksLeft = numBlocks;

	// Allocate memory for extents
	extent* exts = malloc(numBlocks / minContiguous * sizeof(extent));
	if(exts == NULL) 
	{
		DEBUG("malloc failed in getBlocks");
		return NULL;
	}
	int numExts = 0;

	// Iterate through freespace extents  
	// NOTE: This uses the first fit algorithm
	// NOTE: consider best fit instead
	int numFsExtents = (NUM_FREESPACE_BLOCKS * blockSize) / sizeof(extent);
	for(int i = 0; i < numFsExtents; i++) {
		// Check for end of valid freespace extents  
		if(freespaceExtsGlobal[i].location == 0) {
			break;
		}

		// Skip unused/invalid extents  
		if(freespaceExtsGlobal[i].length == 0) {
			continue;
		}

		// Covered all requested blocks  
		if(blocksLeft <= 0) {
			break;
		}
		
		// Find large enough extent, disregarding minimum contiguous requirement if
		// at last extent  
		int searchThreshold = (blocksLeft < minContiguous) ? blocksLeft : minContiguous;
		if(freespaceExtsGlobal[i].length >= searchThreshold) {
			// Set depending on if at last extent  
			int toTake = (blocksLeft < freespaceExtsGlobal[i].length) ? 
				blocksLeft : freespaceExtsGlobal[i].length;

			// Add to returned array  
			exts[numExts].location = freespaceExtsGlobal[i].location;
			exts[numExts].length = toTake;
			numExts++;

			// Shrink freespace extent  
			freespaceExtsGlobal[i].location += toTake;
			freespaceExtsGlobal[i].length -= toTake;  

			blocksLeft -= toTake;
		}
	}

	if(blocksLeft <= 0) {
		// Write updated extents to disk
		if(LBAwrite(freespaceExtsGlobal, NUM_FREESPACE_BLOCKS, VCBGlobal->freespaceLocation) 
			!= NUM_FREESPACE_BLOCKS) 
		{
			DEBUG("LBAwrite failed in getBlocks");
			free(exts);
			exts = NULL;
			return NULL;
		}

		// Set sentinel at end of array  
		exts[numExts].location = 0;
		exts[numExts].length = 0;
		return exts;
	} else {
		// No space available  
		free(exts);
		exts = NULL;
		return NULL;
	}
}