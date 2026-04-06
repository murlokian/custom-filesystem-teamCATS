/**************************************************************
* Class::  CSC-415-03 Fall 2026
* Name::  Allie Young
* Student IDs::  923129607
* GitHub-Name::  milo-pa
* Group-Name::  CATS
* Project:: Basic File System
*
* File:: vcb.c
*
* Description::  This is the volume control block struct and related functions.  
*
**************************************************************/

#include "vcb.h"
#include <stddef.h> // for returning NULL
#include <stdio.h>

#include "fsLow.h"

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

    // Set global vcb and block size  
    VCBGlobal = vcbBuffer;
    blockSizeGlobal = vcbBuffer->blockSize;

    // Return runtime vcb  
    return vcbBuffer;
}

int initVCB(volume_control_block *vcb, uint64_t numBlocks, uint32_t sig, uint16_t blockSize,
            uint8_t rootLoc, uint8_t rootLen, uint8_t fsLoc, uint8_t fsLen) {
    
    // Null check  
    if(vcb == NULL) {
        return -1;
    }

    // Set runtime vcb with parameters  
    vcb->blockCount = numBlocks;
    vcb->signature = sig;
    vcb->blockSize = blockSize;
    vcb->rootLocation = rootLoc;
    vcb->rootLength = rootLen;
    vcb->freespaceLocation = fsLoc;
    vcb->freespaceLength = fsLen;

    // Set global vcb and block size  
    VCBGlobal = vcb;
    blockSizeGlobal = blockSize;

    return 0;
}

bool isVCBOurs(const volume_control_block *vcb) {
    // Null check  
    if(vcb == NULL) {
        return false;
    }

    return vcb->signature == OUR_SIGNATURE;
}
