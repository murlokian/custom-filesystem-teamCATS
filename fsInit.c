/**************************************************************
* Class::  CSC-415-03 Spring 2026
* Name:: Rosa Melton, Milo Pesce Ares, Patricia Ann Payumo, Allie Young
* Student IDs:: 921337778, 921926821, 923555591, 923129607
* GitHub-Name:: milo-pa
* Group-Name:: CATS
* Project:: Basic File System
*
* File:: fsInit.c
*
* Description:: Main driver for file system assignment.
*
* This file is where you will start and initialize your system
*
**************************************************************/

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>

#include "fsLow.h"
#include "mfs.h"
#include "fsVCB.h"
#include "fsStorage.h"
#include "fsDir.h"
#include "debug.h"

int initFileSystem (uint64_t numberOfBlocks, uint64_t blockSize)
	{
	printf ("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);

	// Load the VCB into memory so we can check the signature 
	// and if it doesn't match, we initialize the free space and root directory
	// VCB is loaded into a global variable so it can be accessed by other 
	// functions that need to read/write the VCB
	VCBGlobal = loadVCB(blockSize);
	if (VCBGlobal == NULL) 
	{
		DEBUG("loadVCB failed");
		return -1;
	}

	DEBUG("VCB loaded successfully");

	// Check if VCB is formatted using its signature. 
	// If it matches, exit early with success as volume is already formatted to our system.
	// If not, initialize file system.
	if (VCBGlobal->signature == OUR_SIGNATURE) 
	{
		DEBUG("Volume already formatted. VCB signature matches");
		if (loadFreeSpace() != 0) 
		{
			DEBUG("loadFreeSpace failed");
			return -1;
		}

		rootGlobal = loadDir(NULL);
		if(rootGlobal == NULL) {
			DEBUG("loadDir failed for root");
			return -1;
		}
		cwdGlobal = rootGlobal;

		return 0;
	}

	DEBUG("Volume not formatted, VCB signature does not match");
	DEBUG("Initializing file system...");

	// Fill in the necessary information in the global VCB to be accessed by initFreeSpace 
	// and createDirectory, which will later be assigned to other members in VBCGLobal

	VCBGlobal->blockCount = numberOfBlocks;
	VCBGlobal->blockSize = blockSize;
	VCBGlobal->signature = OUR_SIGNATURE;

	// Initialize free space and assign to VCBGlobal
	int freeSpaceStart = initFreeSpace(numberOfBlocks);
    if (freeSpaceStart < 0) 
	{
		DEBUG("initFreeSpace failed");
        return -1;
    }
	VCBGlobal->freespaceLocation = freeSpaceStart;

	DEBUG("Free space initialized successfully");

	// Initialize root directory and assign its location in VCBGlobal, which
	// ensures root can easily be accessed/loaded into memory from this location 
	// NOTE: passing NULL to have parent directory point to self
	directory_entry *root = createDirectory(DEFAULT_NUM_DIR_ENTRIES, NULL);
	if(root == NULL)
	{
		DEBUG("createDirectory failed");
		return -1;
	}

    int rootLoc = root[0].exts[0].location;
	if (rootLoc < 0) 
	{
		DEBUG("createDirectory failed to allocate blocks for root directory");
        return -1;
    }
	VCBGlobal->rootLocation = rootLoc;
	rootGlobal = root;
	root = NULL;
	cwdGlobal = rootGlobal;

	DEBUG("Root directory created successfully");

	// Once completely populated, write VCB to block VCB_BLOCK_LOCATION on disk (block 0)
	int memoryNeeded = sizeof(volume_control_block);
	int blocksNeeded = (memoryNeeded + blockSize - 1) / blockSize;
    if (LBAwrite(VCBGlobal, blocksNeeded, VCB_BLOCK_LOCATION) != blocksNeeded)
    {
		DEBUG("LBAwrite failed for VCB");
        return -1;
    }

	DEBUG("VCB written to disk successfully. File system initialized");

	// // vvv TESTING CODE vvv
	// // Hardcoding file with "hgello" in it to test cat, growFile()  
	// int newFile = findUnusedDirEntry(rootGlobal);
	// memset(&rootGlobal[newFile], 0, sizeof(directory_entry));

	// rootGlobal[newFile].size = 7;
	// rootGlobal[newFile].entryType = 0;
	// strncpy(rootGlobal[newFile].name, "hgelloFile", MAX_ENTRY_NAME_SIZE - 1);
	// rootGlobal[newFile].name[MAX_ENTRY_NAME_SIZE - 1] = '\0';

	// extent* ext = getBlocks(1, 1);
	// rootGlobal[newFile].exts[0] = ext[0];
	// rootGlobal[newFile].exts[1] = ext[1];
	// free(ext);

	// growFile(&rootGlobal[newFile], 1);
	// DEBUG("size after growing is now %lu", rootGlobal[newFile].size);
	
	// writeDirectory(rootGlobal);

	// LBAwrite("hgello", 1, rootGlobal[newFile].exts[0].location);
	// // ^^^ TESTING CODE ^^^

	return 0;

}
void exitFileSystem ()
	{
	printf ("System exiting\n");

	// Free used resources  
	if(VCBGlobal != NULL) {
		free(VCBGlobal);
		VCBGlobal = NULL;
	}

	if(cwdGlobal != NULL && cwdGlobal != rootGlobal) {
		free(cwdGlobal);
		cwdGlobal = NULL;
	}

	if(rootGlobal != NULL) {
		free(rootGlobal);
		rootGlobal= NULL;
	}
	}