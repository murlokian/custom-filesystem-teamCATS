/**************************************************************
* Class::  CSC-415-03 Spring 2026
* Name:: Milo Pesce Ares, Allie Young, Rosa Melton
* Student IDs:: 921296281, 923129607, 921337778
* GitHub-Name:: milo-pa
* Group-Name:: CATS
* Project:: Basic File System
*
* File:: fsFreeSpace.h
*
* Description:: This is a header file for our freespace system.  
*
**************************************************************/

#ifndef FSSTORAGE_H
#define FSSTORAGE_H

#include <stdint.h>
#include <stdbool.h>

// Number of pre-allocated blocks for free space extents. 10 should be more
// than enough for our purposes, but this can be increased if needed.
#define NUM_FREESPACE_BLOCKS 10

// sentinel node to indicate location to secondary extent table = -2 
#define SECONDARY_EXT_SENTINEL ((uint32_t) 0xFFFFFFFE)

// sentinel node to indicate location to tertiary extent table  = -3
#define TERTIARY_EXT_SENTINEL ((uint32_t) 0xFFFFFFFD)

extern bool freeSpaceInitialized;

// 0 for length signifies unused/invalid extent, 0 for location is a sentinel value
// signifying the end of the extent array
typedef struct extent
{
    // location is an unsigned 64-bit value which is the location of a starting
    // block of contiguous free space
    uint32_t location;

    // length is an unsigned 64-bit value which contains the total amount of blocks
    // that this extent spans
    uint32_t length;
} extent;

// Initialize freespace map with BLOCKCOUNT blocks of size BLOCKSIZE.  
// Returns start block of free space.
int initFreeSpace(uint32_t blockCount);

// Loads free space extents from disk into memory
int loadFreeSpace();

// Allocates NUMBLOCKS blocks in chunks which are at least MINCONTIGUOUS blocks
// (save for last extent). Returns an array of extents
extent* getBlocks(int numBlocks, int minContiguous);

// Frees blocks whose locations are described in extent table EXTS  
// Also returns the freed extents to the freespace system  
int freeBlocks(extent* exts);

// ------------------------- EXTENT FUNCTIONS ------------------------------------

// Somewhat similar logic to strtok_r, helper for multiple calls of b_read(), b_write()
// function updates extentIndex as the 'current position' for the next call (acting as our saveptr)
// iterate from beginning of DE's extent array up to extentIndex and returns updated position
// Updated position to be used when calling LBAread() or LBAwrite()
// NOTE: CALLER IS RESPONSIBLE FOR FREEING RETURNED EXT 
extent * getNextExtent(extent * exts, int * extentIndex);

// traverses through extent array and checks for empty extent sentinel value (0, 0)
int findFirstFree(const extent * exts);

// checks the last index of the primary extent table if the legnth contains 
// our sentinel value for either secondary or tertiary table locations
// this is called to determine which case to use in growFile() 
int checkSentinel(extent ext);

// iterates through an extent table on disk block by block until empty sentinel (0,0) found
// returns the number of extents in the table, -1 on failure
int getTableSize(int tableLoc);

// Removes any empty extents from a list of extents. Handles secondary and
// tertiary tables
extent* cleanExtents(extent* exts);

// called from growFile()
// allocates and writes a new secondary extent table to disk with double the requested size
// and writes the passed extents into the table
int createSecondaryTable(extent * exts, int extCt, int additionalBlocks);

#endif
