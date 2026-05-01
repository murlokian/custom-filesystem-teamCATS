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
	int needsCleanup = 0;
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
		int insertPos = 0;
		int sentinelPos = -1;
		while(insertPos < numFsExtents) {
			// Sentinel check  
			if(freespaceExts[insertPos].location == 0 && freespaceExts[insertPos].length == 0) {
				sentinelPos = insertPos;
				break;
			}

			// Stop at extent immediately to the right  
			if(freespaceExts[insertPos].location > newFreeExt.location) {
				break;
			}

			insertPos++;
		}

		// Find sentinel if not found yet  
		if(sentinelPos == -1) {
			for(int k = insertPos; k < numFsExtents; k++) {
				if(freespaceExts[k].location == 0 && freespaceExts[k].length == 0) {
					sentinelPos = k;
					break;
				}
			}
		}

		// No room in freespace extent table  
		if(sentinelPos == -1) {
			DEBUG("No room in freespace extent table");
			return -1;
		}

		// Check before to handle double merge case  
		int leftMergeable = 0;
		int rightMergeable = 0;
		if(insertPos > 0) {
			if(freespaceExts[insertPos - 1].location + freespaceExts[insertPos - 1].length
				== newFreeExt.location) {
				leftMergeable = 1;
			}
		}
		if(insertPos < sentinelPos) {
			if(newFreeExt.location + newFreeExt.length == freespaceExts[insertPos].location) {
				rightMergeable = 1;
			}
		}

		// Double merge  
		if(leftMergeable && rightMergeable) {
			int rightLoc = freespaceExts[insertPos].location;

			freespaceExts[insertPos - 1].length += 
				newFreeExt.length + freespaceExts[insertPos].length;

			// Set right extent empty  
			freespaceExts[insertPos].length = 0;
			needsCleanup = 1;

			DEBUG("Extent at location %d with %d blocks merged between existing freespace "
				"extents at locations %d and %d for a new extent at location %d with %d blocks",
				newFreeExt.location, newFreeExt.length, freespaceExts[insertPos - 1].location,
				rightLoc, freespaceExts[insertPos - 1].location, 
				freespaceExts[insertPos - 1].length);
		}
		// Left merge  
		else if(leftMergeable) {
			freespaceExts[insertPos - 1].length += newFreeExt.length;

			DEBUG("Extent at location %d with %d blocks merged at end of existing freespace "
				"extent at location %d for a new total of %d blocks", newFreeExt.location,
				newFreeExt.length, freespaceExts[insertPos - 1].location, 
				freespaceExts[insertPos - 1].length);
		}
		// Right merge  
		else if(rightMergeable) {
			int rightLoc = freespaceExts[insertPos].location;

			freespaceExts[insertPos].location = newFreeExt.location;
			freespaceExts[insertPos].length += newFreeExt.length;

			DEBUG("Extent at location %d with %d blocks merged at beginning of existing "
				"freespace extent at location %d for a new location of %d and a new total "
				"of %d blocks", newFreeExt.location, newFreeExt.length, rightLoc, 
				freespaceExts[insertPos].location, freespaceExts[insertPos].length);
		}
		// No merge, insert new extent  
		else {
			// Shift all extents with larger locations to the right starting from sentinel  
			for(int j = sentinelPos; j > insertPos; j--) {
				freespaceExts[j] = freespaceExts[j - 1];
			}

			freespaceExts[insertPos] = newFreeExt;

			DEBUG("Extent inserted into freespace extents at location %d with %d blocks",
				freespaceExts[insertPos].location, freespaceExts[insertPos].length);
		}

		i++;
	}

	// Remove empty extents from double merges  
	if(needsCleanup) {
		cleanExtents(freespaceExts);
	}

	// Write updated extents to disk  
	if(LBAwrite(freespaceExts, NUM_FREESPACE_BLOCKS, VCBGlobal->freespaceLocation) 
		!= NUM_FREESPACE_BLOCKS) 
	{
		DEBUG("LBAwrite failed");
		return -1;
	}

	return 0; 
}

extent *cleanExtents(extent *exts)
{
	if (exts == NULL) {
		DEBUG("exts is NULL");
		return NULL;
	}

	int index = 0;
	int writeIndex = 0;
	int removedCount = 0;
	int numExts = (NUM_FREESPACE_BLOCKS * VCBGlobal->blockSize) / sizeof(extent);

	// Iterate through freespace extents  
	while(index < numExts && exts[index].location != 0) {
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
	}

	memset(&exts[writeIndex], 0, (numExts - writeIndex) * sizeof(extent));

	DEBUG("Cleaned exts: %d removed", removedCount);
	return exts;
}
