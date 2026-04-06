/**************************************************************
* Class::  CSC-415-03 Spring 2026
* Name::  Abha Deshpande, Allie Young
* Student IDs::  [ABHA ID], 923129607
* GitHub-Name::  milo-pa
* Group-Name::  CATS
* Project:: Basic File System
*
* File:: fsFreeSpace.c
*
* Description::  This is the freespace system.  We are using a three-level extent 
* table system to track free space.  Extents will be organized into primary, 
* secondary, and tertiary levels which are created as needed as the disk becomes 
* fragmented and the number of free extents increases.  
*
**************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "fsFreeSpace.h"
#include "fsLow.h"
#include "fsDir.h"
#include "fsVCB.h"

extent* freespaceExtsGlobal = NULL;

int initFreeSpace(uint64_t blockSize, uint64_t blockCount){
    // *** FIX: is this fine hardcoded? will it always be 1? if so, put in define? ***
    VCBGlobal->freespaceLocation = 1;

    // Allocate memory on heap for freespace extents  
    int bytesNeeded = sizeof(extent) * NUM_FREESPACE_EXTS;
    extent* extents = malloc(bytesNeeded);
    if(extents == NULL) {
        perror("malloc failed");
        return -1;
    }

    // Initialize freespace extents  
    int blocksNeeded = (bytesNeeded + blockSize - 1) / blockSize;
    int firstFreeBlock = VCBGlobal->freespaceLocation + blocksNeeded;

    extents[0].location = firstFreeBlock;
    extents[0].length = blockCount - firstFreeBlock;
    for(int i = 1; i < NUM_FREESPACE_EXTS; i++) {
        extents[i].location = 0;
        extents[i].length = 0;
    }

    // Keep runtime extents copy  
    freespaceExtsGlobal = extents;

    // Write extents to disk  
    if(LBAwrite(extents, blocksNeeded, VCBGlobal->freespaceLocation) != blocksNeeded) {
        perror("lbawrite failed");
        free(extents);
        freespaceExtsGlobal = NULL;
        return -1;
    }

    return VCBGlobal->freespaceLocation;
}

// *** FIX: this is currently first fit, maybe change later? ***
int getBlocks(int numBlocks) {
    // Iterate through freespace extents  
    for(int i = 0; i < NUM_FREESPACE_EXTS; i++) {
        // Found large enough extent  
        if(freespaceExtsGlobal[i].length >= numBlocks) {
            int loc = freespaceExtsGlobal[i].location;

            // Shrink freespace extent  
            if(freespaceExtsGlobal[i].length == numBlocks) {
                freespaceExtsGlobal[i].location = 0;
            } else {
                freespaceExtsGlobal[i].location += numBlocks;
            }
            freespaceExtsGlobal[i].length -= numBlocks;

            // Write updated extents to disk  
            int bytesNeeded = sizeof(extent) * NUM_FREESPACE_EXTS;
            int blockSize = VCBGlobal->blockSize;
            int blocksNeeded = (bytesNeeded + blockSize - 1) / blockSize;
            if(LBAwrite(freespaceExtsGlobal, blocksNeeded, VCBGlobal->freespaceLocation) 
                != blocksNeeded) {
                perror("lbawrite failed");
                return -1;
            }

            return loc;
        }
    }
    // No space available  
    return -1;
}
