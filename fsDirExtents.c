/**************************************************************
* Class::  CSC-415-03 Spring 2026
* Name::  Rosa Melton
* Student IDs::  921337778
* GitHub-Name::  milo-pa
* Group-Name::  CATS
* Project:: Basic File System
*
* File:: fsDirExtents.c
*
* Description:: Helper Functions used in traversing and 
*               updating extent tables for directory entries
*
**************************************************************/
#include "fsDir.h"
#include "fsVCB.h"
#include "fsFreeSpace.h"
#include "debug.h"

// sentinel node to indicate location to secondary extent table = -2 
#define SECONDARY_EXT_SENTINEL ((uint32_t) 0xFFFFFFFE)

// sentinel node to indicate location to tertiary extent table  = -3
#define TERTIARY_EXT_SENTINEL ((uint32_t) 0xFFFFFFFD)


// TODO - implement
// traverses all extent tables of a directory entry, returning the next valid extent 
// after *extentIndex and advances it by one position (acting as our 'cursor')
// returns null after iterating through all extents 
extent * getNextExtent(directory_entry * dir, int * extentIndex) {
	DEBUG("Not implemented yet");

    int blockSize = VCBGlobal->blockSize;
    
    while(true) {
        int index = *extentIndex;

        // traversing primary extent array
        if (index < NUM_DIR_EXTENTS) {
            extent *currExt = &dir->exts[index]; 
            (*extentIndex)++;

            // hit empty extent sentinel, END OF EXTS
            if (currExt->location == 0 && currExt->length == 0) {
                return NULL;
            }

            // TODO: check for secondary/tertiary sentinels and read into memory
            // load on demand/cache in static buffer or lazy load ?

            return currExt;
        }
        // TODO: add secondary and tert traversal
    }

    return NULL;
}

// checks current extent array (be it primary, secondary, or tertiary) 
// and returns first instance of sentinel for an empty extent
int findFirstFree(extent* exts) {
    for (int i = 0; i < NUM_DIR_EXTENTS; i++) {
		if (exts[i].location == 0 && exts[i].length == 0) {
			return i;
		}
    }
    return -1;
}

// checks the last index of the primary extent table if the legnth contains 
// our sentinel value for either secondary or tertiary table locations
// this is called to determine which case to use in growFile() 
int checkSentinel(directory_entry * dir) { 
    if (dir->exts[NUM_DIR_EXTENTS - 1].length == SECONDARY_EXT_SENTINEL) { 
        return -2; 
    } else if ( dir->exts[NUM_DIR_EXTENTS - 1].length == TERTIARY_EXT_SENTINEL) {
        return -3; 
    }

    // last index in array holds empty extent sentinel (0, 0)
    return 0; 
}

// Expands the given file by the given number of blocks and updates the directory 
// entry accordingly. Will create secondary extent tables as needed if the number 
// of extents exceeds NUM_DIR_EXTENTS.
// Returns 0 on success, -1 on failure
int growFile(directory_entry * file, int additionalBlocks) {
	DEBUG("Not implemented yet");

	if (file == NULL) {
		DEBUG("file does not exist");
		return -1;
	}
	if (additionalBlocks < 1) {
		DEBUG("cannot add less than one block");
		return -1;
	}

	// get the required blocks from free space first, then populate existing slots or new table
	// ??? if you want 2 extents... why not just alloc + append one extent of N blocks ?
	extent *newExts = getBlocks(additionalBlocks, 1);
	if (newExts == NULL) {
		DEBUG("getBlocks() failed");
		return -1;
	}
	
	// look for the first free/unused ext slot used in case 1 or 2
    int firstFree = findFirstFree(file->exts);

	// look for the sentinel value -2 in length so we know to call either case 2 or 3
    int hasNryTable = 0;
	uint32_t nryTableLoc = checkSentinel(file);

    if (nryTableLoc != 0) { hasNryTable = 1; }

	// 
	int openSlots = NUM_DIR_EXTENTS - firstFree; 


	// iterate over exts array first to find above
	// if we have unused slots, fill those with as many of new exts that we can
	// if overflow, create secondary ext (NEEDS A CHECK)
	// call AddSecondaryExtTable(additionalBlocks) 
	//	  --> calls getBlocks(additionalBlocks, 1)
	

	// THREE CASES TO HANDLE
	// 1) enough free slots to use w/o creating another table
	// 2) can fill to max slots in primary, create secondary
	// 3) add straight to secondary (secondary handles creation of tertiary?)
	
	// HAVE HELPER FUNCTIONS FOR HANDLING EACH 
	// ex) check for when contiguous extents can be merged THEN create tables 
	// ***ADAPTIVE DEFAULT** for number of blocks to add based on the average num
	// of blocks in the last N files in current directory 
	
	// consider adding a trim() for excess unused blocks that would be allocated above
	// this would be a "partial free" ; does freeBlocks() NEED an array of extents?
	// consider changing freeBlocks to take in starting extent and num blocks to loop over 
	// number extents to cover these blocks 

	return -1;
}