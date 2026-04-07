/**************************************************************
* Class::  CSC-415-03 Spring 2026
* Name:: Rosa Melton, Milo Pesce Ares, Patricia Ann Payumo
* Student IDs:: 921337778, 921926821, 923555591
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
#include "fsFreeSpace.h"

// COMMENTED OUT SO WE CAN COMPILE. THIS SHOULD BE DONE LAST OR AT LEAST AFTER
// ALL SIGNATURE/DESIGN CHANGES ETC ARE FLUSHED OUT  
int initFileSystem (uint64_t numberOfBlocks, uint64_t blockSize)
	{
	// printf ("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);

	// // allocate memory for VCB pointer, rounding up to the nearest block size
	// int memoryNeeded = sizeof(volume_control_block);
	// int blocksNeeded = (memoryNeeded + blockSize - 1) / blockSize;
	// memoryNeeded = blocksNeeded * blockSize;

	// volume_control_block *VCBptr = malloc(memoryNeeded);

	// if (VCBptr == NULL) {
	// 	perror("malloc failure");
	// 	return -1;
	// }

	// // read block 0 from disk into VCBptr
	// VCBptr = loadVCB();

	// // Check if VCB is formatted using its signature
	// if (VCBptr->signature == OUR_SIGNATURE) {
	// 	printf("Volume already formatted. VCB signature matches.\n");
	// 	return 0;
	// }

	// printf("Volume not formatted. Initializing VCB.\n");

	// // init root dir ()
	// // TODO: Trish

	// // init free space ()
	// // TODO: Assign the returned valued to the initVCB().
	// // function is currently missing parameter for that

	// // initialize free space
	// int freeSpaceStart = initFreeSpace(blockSize, numberOfBlocks);
    // if (freeSpaceStart < 0)
    // {
    //     printf("Failed to initialize free space.\n");
    //     free(VCBptr);
    //     return -1;
    // }

	// // initialize root directory
	// int rootLoc = initRootDirectory(blockSize);
    // if (rootLoc < 0)
    // {
    //     printf("Failed to initialize root directory.\n");
    //     free(VCBptr);
    //     return -1;
    // }

	// // initialize VCB with file sys metadata
	// if (initVCB(VCBptr, OUR_SIGNATURE, numberOfBlocks, rootLoc, blockSize) != 0)
    // {
    //     printf("Failed to initialize VCB.\n");
    //     free(VCBptr);
    //     return -1;
    // }

	// // write VCB block 0 
    // if (LBAwrite(VCBptr, blocksNeeded, 0) != blocksNeeded)
    // {
    //     printf("Failed to write VCB to disk.\n");
    //     free(VCBptr);
    //     return -1;
    // }

    return 0;
}

void exitFileSystem ()
	{
	printf ("System exiting\n");

	// Free the global VCB variable if it has been allocated
	if (VCBGlobal != NULL)
		free(VCBGlobal);
	}