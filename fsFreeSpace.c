/**************************************************************
* Class::  CSC-415-0# Fall 2025
* Name::
* Student IDs::
* GitHub-Name::
* Group-Name::
* Project:: Basic File System
*
* File:: <name of this file>
*
* Description::
*
**************************************************************/
#include <stdio.h>
#include <stdint.h>
#include "mfs.h"
#include "fsFreeSpace.h"
#include "fsDir.h"
//init function needs to malloc memory
//set extents
//Write it to disk with LBA write 


int initFreeSpace(uint16_t blockSize, uint64_t blockCount){
    //malloc space for extent array 
    extent *extents = malloc(sizeof(extent)*10);

    //zero out all extents so unused entries are clean and detectable
    memset(extents, 0, sizeof(extent) * 10);

    
    //assign each variable
    extents[0].location = 6;
    extents[0].length = blockCount - 6;

    //LBAwrite to write the array to disk
    LBAwrite(extents, (sizeof(extent)*10 + blockSize - 1) / blockSize, 1);
    //return the start block so VCB can store it
    return 1;
   

    
}
