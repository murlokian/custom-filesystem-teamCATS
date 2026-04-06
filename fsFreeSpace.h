/**************************************************************
* Class::  CSC-415-03 Spring 2026
* Name:: Milo Pesce Ares
* Student IDs:: 921296281
* GitHub-Name:: milo-pa
* Group-Name:: CATS
* Project:: Basic File System
*
* File:: freeSpace.h
*
* Description:: header for freeSpace.h
*
**************************************************************/

#ifndef FSFREE_SPACE_H
#define FSFREE_SPACE_H

#include <stdint.h>

typedef struct extent {  // 8 bytes
	
   // location is an unsigned 32-bit value which is the location of a starting
   // block of contiguous free space
   uint32_t location;
  
   // length is an unsigned 32-bit value which contains the 
   // total amount of bytes that this extent spans
   uint32_t length;

} extent;

int initFreeSpace(uint64_t blockSize, uint64_t blockCount);

// Allocates NUMBLOCKS contiguous blocks if possible. Returns disk address to extent table  
int getBlocks(int numBlocks);

#endif