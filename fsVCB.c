/**************************************************************
* Class::  CSC-415-03 Fall 2026
* Name::  Allie Young
* Student IDs::  923129607
* GitHub-Name::  milo-pa
* Group-Name::  CATS
* Project:: Basic File System
*
* File:: fsVCB.c
*
* Description::  This is the volume control block struct and related functions.  
*
**************************************************************/

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "fsVCB.h"
#include "fsLow.h"
#include "debug.h"

volume_control_block* VCBGlobal = NULL;

volume_control_block *loadVCB(uint64_t blockSize) {
	
	// Allocate memory for VCB pointer, rounding up to the nearest block size
	// so that it can be passed to LBAread and LBAwrite
   	int memoryNeeded = sizeof(volume_control_block);
	int blocksNeeded = (memoryNeeded + blockSize - 1) / blockSize;
	memoryNeeded = blocksNeeded * blockSize;

	volume_control_block* VCBBuffer = malloc(memoryNeeded);
	
	if(VCBBuffer == NULL) 
	{
		DEBUG_ERR("malloc failed: fsVCB.c\n");
		return NULL;
	}

	// Read VCB_BLOCK_LOCATION from disk  
	if(LBAread(VCBBuffer, 1, VCB_BLOCK_LOCATION) <= 0) 
	{
		DEBUG("LBAread failed: fsVCB.c\n");
		free(VCBBuffer);
		VCBBuffer = NULL;
	}

	return VCBBuffer;
}
