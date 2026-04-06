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

int initFreeSpace(uint64_t blockSize, uint64_t blockCount);

// Allocates NUMBLOCKS contiguous blocks if possible. Returns disk address to extent table  
int getBlocks(int numBlocks);

#endif