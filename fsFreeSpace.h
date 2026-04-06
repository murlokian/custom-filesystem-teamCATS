/**************************************************************
* Class::  CSC-415-03 Spring 2026
* Name:: Milo Pesce Ares, Allie Young
* Student IDs:: 921296281, 923129607
* GitHub-Name:: milo-pa
* Group-Name:: CATS
* Project:: Basic File System
*
* File:: fsFreeSpace.h
*
* Description:: This is a header file for our freespace system.  
*
**************************************************************/

#ifndef FSFREESPACE_H
#define FSFREESPACE_H

#include <stdint.h>

#include "fsDir.h"

#define NUM_FREESPACE_EXTS 10

extern extent* freespaceExtsGlobal;

// Initialize freespace map with BLOCKCOUNT blocks of size BLOCKSIZE.  
// Returns start block of free space.  
int initFreeSpace(uint64_t blockSize, uint64_t blockCount);

// *** FIX: what if there is enough but not contiguous. could possibly have a mergeFreespace
// which is called if getBlocks fails (-1) then call getBlocks again ***
// Allocates NUMBLOCKS contiguous blocks if possible. Returns disk address to extent  
int getBlocks(int numBlocks);

#endif