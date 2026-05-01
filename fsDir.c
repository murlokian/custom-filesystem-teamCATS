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
#include "fsLow.h"
#include "fsVCB.h"
#include "debug.h"

directory_entry* rootGlobal = NULL;
directory_entry* cwdGlobal = NULL;

static void printExts(directory_entry * file);

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
	for(int i = 0; exts[i].length != 0; i++) {
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
	for(int i = 0; parent[0].exts[i].length != 0; i++) {
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
	if (LBAwrite(dir, blocksNeeded, dir[0].exts[0].location) != blocksNeeded)
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

	// Iterate through extents  
	int offset = 0;
	int extIndex = 0;
	extent* ext;
	while(true) {
		ext = getNextExtent(dir->exts, &extIndex);

		// No more extents  
		if(ext == NULL) {
			break;
		}

		// Load into in memory array  
		if(LBAread((char*)deez + offset, ext->length, ext->location) != ext->length) {
			DEBUG("lbaread failed");
			free(deez);
			deez = NULL;
			return NULL;
		}
		
		offset += ext->length * VCBGlobal->blockSize;
	}

	DEBUG("Loaded directory with size %d blocks", numBlocks);
	return deez;
}

// Expands the given file by the given number of blocks and updates the directory 
// entry accordingly. Will create secondary extent tables as needed if the number 
// of extents exceeds NUM_DIR_EXTENTS.
// Returns 0 on success, -1 on failure
int growFile(directory_entry * file, int additionalBlocks) {
	DEBUG("implemented up to secondary exts");

	if (file == NULL) {
		DEBUG("file does not exist");
		return -1;
	}
	if (additionalBlocks < 1) {
		DEBUG("cannot add less than one block");
		return -1;
	}

	int blockSize = VCBGlobal->blockSize;

	// get the required blocks from free space first, then populate existing slots or new table
	extent *newExts = getBlocks(additionalBlocks, 1);
	if (newExts == NULL) {
		DEBUG("getBlocks() failed");
		return -1;
	}
	// count how many extents returned (stopping at empty ext sentinel)
    int newExtCount = 0;
    while (checkSentinel(newExts[newExtCount])!= 0) {
       newExtCount++;
    }

	DEBUG("adding %d exts to file %s", newExtCount, file->name);
	int newExtBlocks = (newExtCount * sizeof(extent) + (blockSize - 1)) / blockSize;
	
	// look for the first free/unused ext slot used in case 1 or 2
    int firstFree = findFirstFree(file->exts);

	int lastPrimaryIndex = NUM_DIR_EXTENTS - 1;
	extent * lastPrimaryExt = &file->exts[lastPrimaryIndex];

	// if table has any sentinel present, know to call either case 2 or 3
	
	int nryTableLoc = lastPrimaryExt->location ;
	int sentinel = checkSentinel(file->exts[lastPrimaryIndex]);

	// CASE 1 - free slots available in primary table --------------------------------------
	// fill and create secondary if necessary
	if (firstFree != -1 && firstFree < lastPrimaryIndex) {
		int openSlots = lastPrimaryIndex - firstFree;

		// CASE 1A - all new extents fit in primary
		if (newExtCount <= openSlots) {
			for (int i = 0; i < newExtCount; i++) {
				file->exts[firstFree + i] = newExts[i];
			}

			printExts(file);

			free(newExts);
			return 0;
		}

		// CASE 1B - fill what fits in primary --> put remainder into new secondary
		int toFill = openSlots;
		for (int i = 0; i < toFill; i++) {
			file->exts[firstFree + i] = newExts[i];
		}

		int remaining = newExtCount - toFill;

		// assign starting loc of secondary ext array
		// to position in new extent array that was just added to primary
		extent *remainingExts = &newExts[toFill];

		// allocate enough blocks for secondary table creation
		// new tables will be double the size the caller requires
		int secLoc = createSecondaryTable(remainingExts, remaining, additionalBlocks);
		if (secLoc == -1) {
			DEBUG("createSecondaryTable failed");
			free(newExts);
			return -1;
		}

		// write new secondary table to disk starting at 
		// the first block returned from getBlocks(), 
		// write <remaining> num blocks to volume
		if (LBAwrite(remainingExts, remaining, secLoc) != remaining) {
			DEBUG("LBAwrite failed for secondary table");
			free(newExts);
			return -1;
		}

		// change last ext in primary to secondary loc + sentinel value to link the tables
		lastPrimaryExt->location = secLoc;
		lastPrimaryExt->length = SECONDARY_EXT_SENTINEL;

		printExts(file);

		free(newExts);
		return 0; 
	} // END CASE 1A + 1B

	// CASE 2 - primary full, add new exts immediately to new secondary table 
	// 						  IF tertiary/secondary does not exist
	if (sentinel >= 0) {
		DEBUG("primary full, creating secondary table");
		lastPrimaryExt->location = createSecondaryTable(newExts, 
														newExtCount, 
														additionalBlocks);
		lastPrimaryExt->length = SECONDARY_EXT_SENTINEL;
		
		if (lastPrimaryExt->location == -1) {
			DEBUG("createSecondaryTable failed");
			free(newExts);
			return -1;
		}

		free(newExts);

		printExts(file);
		return 0;
	}

	// TODO - add check tertiary sentinel here

	// CASE 3 - (A) secondary exists, fill available slots within 
	//          (B) and/or create tert + another sec
	if (sentinel == -2) {
		int existingSecLoc = lastPrimaryExt->location;

		// get num existing exts in secondary to 
		int secExtCount = getTableSize(existingSecLoc);

		if (secExtCount == -1) {
			DEBUG("getTableSize failed for secondary table");

			free(newExts);
			return -1;
		}
		
		// get size of existing secondary ext array
		int secTableBlocks = (secExtCount * sizeof(extent) + (blockSize - 1)) / blockSize;

		// load secondary to call findFirstFree()
		// need offset to start appending additional exts
		extent * secBuf = malloc(secTableBlocks * blockSize);
		if (secBuf == NULL) {
			DEBUG_ERR("malloc failed for secondary table buffer");
			free(newExts);
			return -1;
		}

		if (LBAread(secBuf,secTableBlocks, existingSecLoc) != secTableBlocks) {
			DEBUG("LBAread failed for secondary table");
			free(secBuf);
			free(newExts);
			return -1;
		}

		int secFirstFree = findFirstFree(secBuf);
		free(secBuf);


		// CASE 3 - room in existing secondary, fill and exit
		if (secFirstFree != -1) {
			int slotsPerBlock = blockSize / sizeof(extent);
			int extWritten = 0;

			// iterate block by block starting from the block containing secFirstFree
			int secByteOffset = secFirstFree * sizeof(extent);
			int blockOffset = secByteOffset / blockSize;
			int slotInBlock = (secByteOffset % blockSize) / sizeof(extent);

			while (extWritten < newExtCount) {
				// read existing block
				extent *writeBuf = malloc(blockSize);
				if (writeBuf == NULL) {
					DEBUG_ERR("malloc failed for write buffer");
					free(newExts);
					return -1;
				}
				if (LBAread(writeBuf, 1, existingSecLoc + blockOffset) != 1) {
					DEBUG("LBAread failed reading block %d", blockOffset);
					free(writeBuf);
					free(newExts);
					return -1;
				}

				// fill slots in this block starting at slotInBlock
				int slotsAvailable = slotsPerBlock - slotInBlock;
				int toFill;

				if (newExtCount - extWritten <= slotsAvailable) {
					toFill = newExtCount - extWritten;
				} else {
					toFill = slotsAvailable;
				}

				for (int i = 0; i < toFill; i++) {
					writeBuf[slotInBlock + i] = newExts[extWritten + i];
				}

				if (LBAwrite(writeBuf, 1, existingSecLoc + blockOffset) != 1) {
					DEBUG("LBAwrite failed for block %d", blockOffset);
					free(writeBuf);
					free(newExts);
					return -1;
				}
				free(writeBuf);

				extWritten += toFill;
				blockOffset++;
				slotInBlock = 0; // subsequent blocks start at slot 0
			}

			free(newExts);

			printExts(file);
			return 0;
		}
	}
	return -1;
}

static void printExts(directory_entry * file) {
	DEBUG("\n");
    DEBUG("=== extents for '%s' (size: %lu) ===", file->name, file->size);

    int extentIndex = 0;
    int count = 0;
    extent * ext;

    while ((ext = getNextExtent(file->exts, &extentIndex)) != NULL) {
        DEBUG("  ext[%d]: loc=%u len=%u", count, ext->location, ext->length);
        freeExt(ext, file->exts);
        count++;
    }

    if (count == 0) {
        DEBUG("  no extents found");
    }

    DEBUG("=== end extents (%d total) ===\n", count);
}
