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
* Description::  This is the freespace system.  We are using an extent system
* to track free space.  Extents will be organized into primary and secondary 
* levels which are created as needed as the disk becomes fragmented and the 
* number of free extents increases.  
*
**************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "fsStorage.h"
#include "fsLow.h"
#include "fsDir.h"
#include "fsVCB.h"
#include "debug.h"

extent* freespaceExts = NULL;
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
	memset(extents + 1, 0, bytesNeeded - sizeof(extent));

	// Keep runtime extents copy
	freespaceExts = extents;

	// Write extents to disk  
	int LBAblocks = LBAwrite(extents, NUM_FREESPACE_BLOCKS, freespaceLocation); 
	if(LBAblocks != NUM_FREESPACE_BLOCKS) 
	{
		DEBUG("LBAwrite failed in initFreeSpace");
		free(freespaceExts);
		freespaceExts = NULL;
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
	freespaceExts = extents;
	freeSpaceInitialized = true;

	DEBUG("Free space loaded successfully");
	DEBUG("Free Space extent one: location %d, length %d", 
		freespaceExts[0].location, freespaceExts[0].length);

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
	extent* exts = calloc(numBlocks / minContiguous, sizeof(extent));
	if(exts == NULL) 
	{
		DEBUG_ERR("malloc failed in getBlocks");
		return NULL;
	}
	int numExts = 0;

	// Iterate through freespace extents  
	// NOTE: This uses the first fit algorithm
	int numFsExtents = (NUM_FREESPACE_BLOCKS * blockSize) / sizeof(extent);
	for(int i = 0; i < numFsExtents; i++) {
		// Check for end of valid freespace extents  
		if(freespaceExts[i].location == 0) {
			break;
		}

		// Skip unused/invalid extents  
		if(freespaceExts[i].length == 0) {
			continue;
		}

		// Covered all requested blocks  
		if(blocksLeft <= 0) {
			break;
		}
		
		// Find large enough extent, disregarding minimum contiguous requirement if
		// at last extent  
		int searchThreshold = (blocksLeft < minContiguous) ? blocksLeft : minContiguous;
		if(freespaceExts[i].length >= searchThreshold) {
			// Set depending on if at last extent  
			int toTake = (blocksLeft < freespaceExts[i].length) ? 
				blocksLeft : freespaceExts[i].length;

			// Add to returned array  
			exts[numExts].location = freespaceExts[i].location;
			exts[numExts].length = toTake;
			DEBUG("Extent of %d blocks taken from freespace at location %d", 
				exts[numExts].length, exts[numExts].location);
			numExts++;

			// Shrink freespace extent  
			freespaceExts[i].location += toTake;
			freespaceExts[i].length -= toTake;  

			blocksLeft -= toTake;
		}
	}

	if(blocksLeft <= 0) {
		// Write updated extents to disk
		if(LBAwrite(freespaceExts, NUM_FREESPACE_BLOCKS, VCBGlobal->freespaceLocation) 
			!= NUM_FREESPACE_BLOCKS) 
		{
			DEBUG("LBAwrite failed in getBlocks");
			free(exts);
			exts = NULL;
			return NULL;
		}

		// Allocate additional space for sentinel extent if needed  
		if(numExts == numBlocks / minContiguous) {
			exts = realloc(exts, (numExts + 1) * sizeof(extent));
			if(exts == NULL) {
				DEBUG("realloc failed in getBlocks");
				return NULL;
			}
			numExts++;
		}

		// Set sentinel at end of array  
		exts[numExts - 1].location = 0;
		exts[numExts - 1].length = 0;

		DEBUG("Returning %d extents", numExts);
		return exts;
	} else {
		// No space available  
		free(exts);
		exts = NULL;
		DEBUG("No space available");
		return NULL;
	}
}

int freeBlocks(extent* exts) {
	// Iterate through extents to be freed  
	int i = 0;
	while(exts[i].location != 0) {
		// Skip already marked free extents  
		if(exts[i].length == 0) {
			DEBUG("Skip freeing extent at location %d because length is already zeroed", 
				exts[i].location);
			i++;
			continue;
		}

		// Iterate through freespace extents  
		extent newFreeExt = exts[i];
		int numFsExtents = (NUM_FREESPACE_BLOCKS * VCBGlobal->blockSize) / sizeof(extent);
		for(int j = 0; j < numFsExtents; j++) {
			// Can be merged at end of existing freespace extent  
			if(freespaceExts[j].location + freespaceExts[j].length == newFreeExt.location) {
				freespaceExts[j].length += newFreeExt.length;
				DEBUG("Extent at location %d with %d blocks merged at end of existing freespace \
					extent at location %d for a new total of %d blocks", newFreeExt.location,
					newFreeExt.length, freespaceExts[j].location, freespaceExts[j].length);
				break;
			}

			// Can be merged at beginning of existing freespace extent  
			if(newFreeExt.location + newFreeExt.length == freespaceExts[j].location) {
				freespaceExts[j].location = newFreeExt.location;
				freespaceExts[j].length += newFreeExt.length;
				DEBUG("Extent at location %d with %d blocks merged at beginning of existing \
					freespace extent at location %d for a new location of %d and a new total \
					of %d blocks", newFreeExt.location, newFreeExt.length, 
					freespaceExts[j].location, newFreeExt.location, freespaceExts[j].length);
				break;
			}

			// *** FIX: insert sorted ***
			// Add at end of freespace extents  
			if(freespaceExts[j].location != 0) {
				continue;
			}
			freespaceExts[j] = newFreeExt;
			DEBUG("Extent of %d blocks added to end of freespace extents at location %d", 
				freespaceExts[j].length, freespaceExts[j].location);
			break;
		}

		i++;
	}

	// Write updated extents to disk  
	if(LBAwrite(freespaceExts, NUM_FREESPACE_BLOCKS, VCBGlobal->freespaceLocation) 
		!= NUM_FREESPACE_BLOCKS) 
	{
		DEBUG("LBAwrite failed");
		return 1;
	}

	return 0; 
}

extent *cleanExtents(extent *exts)
{

	int index = 0;
	int writeIndex = 0;
	int removedCount = 0;

	extent *currExt = getNextExtent(exts, &index);

	while (currExt != NULL)
	{
		if (exts[index].length == 0)
		{
			removedCount++;
		}
		else
		{
			exts[writeIndex] = exts[index];
			writeIndex++;
		}
		index++;
		currExt = getNextExtent(exts, &index);
	}

	memset(&exts[writeIndex], 0, (index - writeIndex) * sizeof(extent));

	DEBUG("Cleaned exts: %d removed", removedCount);
	return exts;
}
