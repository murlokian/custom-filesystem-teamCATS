/**************************************************************
* Class::  CSC-415-03 Spring 2026
* Name::  Rosa Melton
* Student IDs::  921337778
* GitHub-Name::  milo-pa
* Group-Name::  CATS
* Project:: Basic File System
*
* File:: fsStorageExts.c
*
* Description:: Helper Functions used in traversing and 
*               updating extent tables for directory entries
*
**************************************************************/

#include "fsDir.h"
#include "fsVCB.h"
#include "fsStorage.h"
#include "debug.h"

// sentinel node to indicate location to secondary extent table = -2 
#define SECONDARY_EXT_SENTINEL ((uint32_t) 0xFFFFFFFE)

// sentinel node to indicate location to tertiary extent table  = -3
#define TERTIARY_EXT_SENTINEL ((uint32_t) 0xFFFFFFFD)


// TODO - implement
// traverses all extent tables of a directory entry, returning the next valid extent 
// after *extentIndex and advances it by one position (acting as our 'cursor')
// returns null after iterating through all extents 
extent * getNextExtent(const extent * exts, int * extentIndex) {
	DEBUG("implemented for primary traversal only");

    // for loading secondary or tertiary tables
	int blockSize = VCBGlobal->blockSize;

    while(true) {
        int index = *extentIndex;

        // traversing primary extent array
        if (index < NUM_DIR_EXTENTS) {
            extent *currExt = &exts[index]; 
            (*extentIndex)++;

			// check for sentinel if secondary/tertiary present 
			int sentinelVal = checkSentinel(*currExt);

            // hit empty extent sentinel, END OF EXTS
            if (sentinelVal == 0) {
				DEBUG("iterated to end of table");
                return NULL;
            }
			// SKIP next extent if empty/freed (fragmentation check)
			if (currExt->length == 0) {
				DEBUG("skipping freed extent (length 0)");

				continue;
				// TODO: move extentIndex by 2 positions
			}

            // TODO: check for secondary/tertiary sentinels and read into memory
            // load on demand/cache in static buffer or lazy load ?
			// LAZY LOAD because diff fds of b_read() and b_write() will be calling
			

            return currExt;
        }
        // TODO: add secondary and tert traversal
    }

    return NULL;
}

// checks current extent array (be it primary, secondary, or tertiary) 
// and returns first instance of sentinel for an empty extent
int findFirstFree(const extent * exts) {

    for (int i = 0; i < NUM_DIR_EXTENTS; i++) {
		if (exts[i].location == 0 && exts[i].length == 0) {
			DEBUG("returning at %d index, empty extent at location %d", i, exts[i].location);
			return i;
		}
    }

    return -1;
}

// checks the last index of the PRIMARY extent table if the length contains 
// our sentinel value for either secondary or tertiary table locations
// this is called to determine which case to use in growFile() 
int checkSentinel(extent ext) { 
	switch (ext.length) {
		case 0:
			if (ext.location == 0) {
				DEBUG("returning empty ext sentinel");
				return 0;
			}
			else { 
				DEBUG("passed ext is not a sentinel");
				return -1;
			}

		case SECONDARY_EXT_SENTINEL:
			DEBUG("returning secondary sentinel value");
        	return -2; 

    	case TERTIARY_EXT_SENTINEL:
			DEBUG("returning tertiary sentinel value");
			return -3;	

		default:
			DEBUG("passed ext is not a sentinel");
			return -1;
    }
}
