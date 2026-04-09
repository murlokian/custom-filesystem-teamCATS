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
#include <stdbool.h>

#include "fsDir.h"

#define NUM_FREESPACE_BLOCKS 10

extern bool freeSpaceInitialized;

// Initialize freespace map with BLOCKCOUNT blocks of size BLOCKSIZE.  
// Returns start block of free space.
int initFreeSpace(uint64_t blockCount);

// Allocates NUMBLOCKS blocks in chunks which are at least MINCONTIGUOUS blocks
// (save for last extent). Returns an array of extents  
extent* getBlocks(int numBlocks, int minContiguous);

#endif
