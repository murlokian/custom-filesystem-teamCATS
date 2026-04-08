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

int loadVCB(uint64_t blockSize) {
    puts("entered loadVCB()\n");
    fflush(stdout);

    // allocate memory for VCB pointer, rounding up to the nearest block size
   	int memoryNeeded = sizeof(volume_control_block);
	int blocksNeeded = (memoryNeeded + blockSize - 1) / blockSize;
	memoryNeeded = blocksNeeded * blockSize;

    volume_control_block* vcbBuffer = malloc(memoryNeeded);

    //printf("vcbBuffer location:%d\n", vcbBuffer);
    
    if(vcbBuffer == NULL) {
        perror("malloc failed: fsVCB.c\n");
        return -1;
    }

    // Read block 0 from disk  
    if(LBAread(vcbBuffer, 1, 0) <= 0) {
        fprintf(stderr, "LBAread failed: fsVCB.c\n");
        
        free(vcbBuffer);
        return -1;
    }

    // Set global vcb  
    VCBGlobal = vcbBuffer;
    

    // Return 0 on successful load 
    return 0;
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

