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
#include "fsDir.h"

int initFileSystem (uint64_t numberOfBlocks, uint64_t blockSize)
	{
	printf ("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);

	volume_control_block *VCBLocal;

	VCBLocal = loadVCB(blockSize);
	if (VCBLocal == NULL) {
		fprintf(stderr, "loadVCB failed\n");

		return -1;
	}

	// Check if VCB is formatted using its signature
	// if (VCBLocal->signature == OUR_SIGNATURE) {
	// 	printf("Volume already formatted. VCB signature matches.\n");
		
	// 	// set global VCB and 'discard' VCBLocal
	// 	VCBGlobal = VCBLocal;
	// 	VCBLocal = NULL;

	// 	return 0;
	// }

	printf("Volume not formatted. Initializing VCB.\n");


	//FIXME: getBlocks in fsFreeSpace() segfaulting;
	//       cannot create root without init'ing free space
	// NOTE: initFreeSpace() requires VCBGlobal to be init'd
	//       VCBGlobal cannot be init'd w/o free space and so on...
	//       NEED RESTRUCTURING OF FREE SPACE (AND VCB?) INITS

	// initialize root directory w/ 2 entries: '.' and '..'
	// NULL to have parent directory point to self
	// directory_entry *root = createDirectory(2, NULL, blockSize);
	// if(root == NULL) {
	// 	printf("null after createdir");
    // 	fflush(stdout);
	// }

    // int rootLoc = root[0].exts[0].location;
	// if (rootLoc < 0)
    // {
    //     fprintf(stderr, "Failed to initialize root directory.\n");
		
    //     free(VCBGlobal);
    //     return -1;
    // }

	// initialize VCB with file system metadata
	// FIXME: free space location hard-coded to 1 for now as both initVCB
	// and initFileSystem rely on the other to already be initialized
	// FIXME: rootLoc hard-coded to block 11, fix getBlocks() !
	if (initVCB(VCBLocal, numberOfBlocks, 11,
		 	1 , OUR_SIGNATURE, blockSize) != 0)
    {
        fprintf(stderr, "Failed to initialize VCB.\n");

        free(VCBGlobal);
        return -1;
    }

	// initialize free space
	int freeSpaceStart = initFreeSpace(numberOfBlocks);
    if (freeSpaceStart < 0)
    {
        fprintf(stderr, "Failed to initialize free space.\n");

        free(VCBGlobal);
        return -1;
    }

	// write VCB block 0 
	// FIXME - these 2 lines used in loadVCB(), consolidate them
	//		   better in another [global?] function?
	//		   (or leave at hard-coded 1 bluh)	
	int memoryNeeded = sizeof(volume_control_block);
	int blocksNeeded = (memoryNeeded + blockSize - 1) / blockSize;
    if (LBAwrite(VCBGlobal, blocksNeeded, 0) != blocksNeeded)
    {
        fprintf(stderr, "Failed to write VCB to disk.\n");

        free(VCBGlobal);
        return -1;
    }

    return 0;
}

void exitFileSystem ()
	{
	printf ("System exiting\n");

	// Free the global VCB variable if it has been allocated
	if (VCBGlobal != NULL)
		free(VCBGlobal);
	}