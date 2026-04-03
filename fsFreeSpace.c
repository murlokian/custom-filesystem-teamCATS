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

//init function needs to malloc memory
//set extents
//Write it to disk with LBA write 


int initFreeSpace(uint64_t blockSize, uint64_t blockCount){
    //malloc space for extent array 
    //-- let C figure it out. How many bytes to allocate?
    
    //LBAwrite to write the array to disk
    //return the start block so VCB can store it
    
}
