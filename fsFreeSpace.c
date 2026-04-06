/**************************************************************
* Class::  CSC-415-03 Spring 2026
* Name::
* Student IDs::
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

//init function needs to malloc memory
//set extents
//Write it to disk with LBA write 

int initFreeSpace(uint64_t blockSize, uint64_t blockCount){
    //malloc space for extent array 
    extent *extents = malloc(sizeof(extent)*10);
    
    //assign each variable
    extents[0].location = 6;
    extents[0].length = blockCount - 6;

    //LBAwrite to write the array to disk
    LBAwrite(extents, (sizeof(extent)*10 + blockSize - 1) / blockSize, 1);
    //return the start block so VCB can store it
    
    // TODO: free the malloced space
}

int getBlocks(int numBlocks) {
    return 0;
}
