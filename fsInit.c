/**************************************************************
* Class::  CSC-415-03 Spring 2026
* Name:: Rosa Melton, Milo Pesce Ares
* Student IDs:: 921337778, 921926821
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
#include "vcb.h"
#include "fsFreeSpace.h"

int initFileSystem (uint64_t numberOfBlocks, uint64_t blockSize)
	{
	printf ("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);

	// allocate memory for VCB pointer, rounding up to the nearest block size
	int memoryNeeded = sizeof(volume_control_block);
	int blocksNeeded = (memoryNeeded + blockSize - 1) / blockSize;
	memoryNeeded = blocksNeeded * blockSize;

	volume_control_block *VCBptr = malloc(memoryNeeded);

	if (VCBptr == NULL) {
		perror("malloc failure");
		return -1;
	}

	// read block 0 from disk into VCBptr
	VCBptr = loadVCB();

	// Check if VCB is formatted using its signature
	if (isVCBOurs(VCBptr)) {
		printf("Volume already formatted. VCB signature matches.\n");
		return 0;
	}

	printf("Volume not formatted. Initializing VCB.\n");

	// init root dir ()
	// TODO: Trish

	// init free space ()
	// TODO: Assign the returned valued to the initVCB().
	// function is currently missing parameter for that
	initFreeSpace(blockSize, numberOfBlocks);

	initVCB(VCBptr, OUR_SIGNATURE, numberOfBlocks, 0, blockSize);

	// write VCB back to disk at block 0
	LBAwrite(VCBptr, blocksNeeded, 0);

	return 0;
	}
	
	
void exitFileSystem ()
	{
	printf ("System exiting\n");

	// Free the global VCB variable if it has been allocated
	if (VCBGlobal != NULL)
		free(VCBGlobal);
	}