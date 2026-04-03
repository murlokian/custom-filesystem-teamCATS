/**************************************************************
* Class::  CSC-415-03 Spring 2026
* Name:: Rosa Melton, 
* Student IDs:: 921337778, 
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

// 	 In fsInit is the initFileSystem function where you want to start this code
//   Malloc a Block of memory as your VCB pointer and LBAread block 0
//   You now have a pointer to a structure, so look at the signature (magic number)
//   in your structure and see if it matches.

//   If it matches you have initialized your volume already – If not you need to
//   initialize it

int initFileSystem (uint64_t numberOfBlocks, uint64_t blockSize)
	{
	printf ("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);
	/* TODO: Add any code you need to initialize your file system. */



	// allocate one block for VCB (LBA block 0)
	volume_control_block *VCBptr = malloc(blockSize); 

	if (VCBptr == NULL) {
		perror("malloc failure"); // consider using calloc instead?
		return -1;
	}

	// check if vcb's signature matches fs's signature
	// return 0 if matching, vcb is already formatted
	// if (VCBptr->signature == SIGNATURE ) 
	// 	{
	// 		return 0; // volume already formatted 
	// 	}


	// init root dir ()

	// init free space ()

	return 0;
	}
	
	
void exitFileSystem ()
	{
	printf ("System exiting\n");

	// free our vcb ptr
	}