/**************************************************************
* Class::  CSC-415-03 Spring 2026
* Name::  Allie Young, Milo Pesce Ares
* Student IDs::  923129607, 921296821
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
bool freeSpaceInitialized = false;

int initFreeSpace(uint64_t blockCount) {

    int blockSize = VCBGlobal->blockSize;
	int blockAfterVCB = 
        VCB_BLOCK_LOCATION +(sizeof(volume_control_block) + blockSize - 1) / blockSize;

    #ifdef DEBUG_FREESPACE
    printf("Address of freespaceLocation: %d\n", blockAfterVCB);
    #endif

    int freespaceLocation = blockAfterVCB;

    // Allocate memory on heap for freespace extents
    int numExtents = (NUM_FREESPACE_BLOCKS * blockSize ) / sizeof(extent);
    int bytesNeeded = numExtents * sizeof(extent);

    extent* extents = malloc(bytesNeeded);
    if(extents == NULL) {
        perror("malloc failed");
        return -1;
    }

    // Initialize freespace extents
    int firstFreeBlock = freespaceLocation + NUM_FREESPACE_BLOCKS;

    extents[0].location = firstFreeBlock;
    extents[0].length = blockCount - firstFreeBlock;
    for(int i = 1; i < numExtents; i++) {
        extents[i].location = 0;
        extents[i].length = 0;
    }

    // Keep runtime extents copy  
    freespaceExtsGlobal = extents;

    // Write extents to disk  
    int LBAblocks = LBAwrite(extents, NUM_FREESPACE_BLOCKS, freespaceLocation); 
    if(LBAblocks != NUM_FREESPACE_BLOCKS) {
        fprintf(stderr, "LBAwrite failed in initFreeSpace\n");
        free(extents);
        freespaceExtsGlobal = NULL;
        return -1;
    }

    freeSpaceInitialized = true;

    return freespaceLocation;
}

// FIXME: change return to array of extents 
int getBlocks(int numBlocks) {
    // Ensure freespace system has been initialized  
    if(!freeSpaceInitialized){
        fprintf(stderr, "Error: Free space system not initialized.\n");
        return -1;
    }

    int blockSize = VCBGlobal->blockSize;

    // Iterate through freespace extents
    // NOTE: This uses the first fit algorithm
    // NOTE: consider best fit instead
    int numExtents = (NUM_FREESPACE_BLOCKS * blockSize) / sizeof(extent);
    for(int i = 0; i < numExtents; i++) {
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
            int bytesNeeded = sizeof(extent) * numExtents;
            int blocksNeeded = (bytesNeeded + blockSize - 1) / blockSize;

            if(LBAwrite(freespaceExtsGlobal, blocksNeeded, VCBGlobal->freespaceLocation) 
                != blocksNeeded) {
                fprintf(stderr, "LBAwrite failed in getBlocks\n");
                return -1;
            }

            return loc;
        }
    }

    // No space available  
    return -1;
}
