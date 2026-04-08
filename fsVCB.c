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

volume_control_block* VCBGlobal = NULL;

volume_control_block *loadVCB(uint64_t blockSize) {
    
    // allocate memory for VCB pointer, rounding up to the nearest block size
   	int memoryNeeded = sizeof(volume_control_block);
	int blocksNeeded = (memoryNeeded + blockSize - 1) / blockSize;
	memoryNeeded = blocksNeeded * blockSize;

    // TODO: volume_control_block size is 32 bytes yet we malloc a block...
    // fix later.............?  
    volume_control_block* VCBBuffer = malloc(memoryNeeded);
    
    if(VCBBuffer == NULL) {
        perror("malloc failed: fsVCB.c\n");
        return NULL;
    }

    // Read block 0 from disk  
    if(LBAread(VCBBuffer, 1, 0) <= 0) {
        fprintf(stderr, "LBAread failed: fsVCB.c\n");
        
        free(VCBBuffer);
        return NULL;
    }

    // Return localVCB to check signature before saving to VCBGlobal 
    return VCBBuffer;
}

int initVCB(volume_control_block *vcb, uint64_t numBlocks, uint8_t rootLoc, 
            uint8_t fsLoc, uint32_t sig, uint16_t blockSize) {
    // Null check  
    if(vcb == NULL) {
        return -1;
    }

    // Set runtime vcb with parameters  
    vcb->blockCount = numBlocks;
    vcb->rootLocation = rootLoc;
    vcb->freespaceLocation = fsLoc;
    vcb->signature = sig;
    vcb->blockSize = blockSize;

    // Set global vcb  
    VCBGlobal = vcb;

    return 0;
}

