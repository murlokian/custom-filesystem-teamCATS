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

volume_control_block* loadVCB() {
    // Allocate memory on heap for vcb  
    volume_control_block* vcbBuffer = malloc(sizeof(volume_control_block));
    if(vcbBuffer == NULL) {
        perror("malloc failed");
        return NULL;
    }

    // Read block 0 from disk  
    if(LBAread(vcbBuffer, 1, 0) <= 0) {
        perror("lbaread failed");
        free(vcbBuffer);
        return NULL;
    }

    // Set global vcb  
    VCBGlobal = vcbBuffer;

    // Return runtime vcb  
    return vcbBuffer;
}

int initVCB(volume_control_block *vcb, uint64_t numBlocks, uint8_t rootLoc, 
            uint8_t fsLoc, uint32_t sig, uint16_t blockSize, uint8_t rootLen) {
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

