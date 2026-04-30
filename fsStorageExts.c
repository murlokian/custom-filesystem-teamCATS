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
#include <stdio.h>
#include <stdlib.h>

#include "fsDir.h"
#include "fsVCB.h"
#include "fsStorage.h"
#include "fsLow.h"
#include "debug.h"

// sentinel node to indicate location to secondary extent table = -2 
#define SECONDARY_EXT_SENTINEL ((uint32_t) 0xFFFFFFFE)

// sentinel node to indicate location to tertiary extent table  = -3
#define TERTIARY_EXT_SENTINEL ((uint32_t) 0xFFFFFFFD)

// traverses all extent tables of a directory entry, returning the next valid extent 
// after *extentIndex and advances it by one position (acting as our 'cursor')
// returns null after iterating through all extents 
// NOTE: CALLER IS RESPONSIBLE FOR FREEING RETURNED EXT 
extent * getNextExtent(extent * exts, int * extentIndex) {
	// DEBUG("secondary/tertiary not yet tested");

    // for loading secondary / tertiary tables
	int blockSize = VCBGlobal->blockSize;
	int sentinelVal;

    while(true) {
        int index = *extentIndex;
		
        // EXPLORING PRIMARY EXTENT TABLE ------------------------------------------------
        if (index < NUM_DIR_EXTENTS) {
            extent * currExt = &exts[index]; 
            (*extentIndex)++;

			// check if current extent contains a sentinel value 
			// (0, -2, or -3; -1 if non-sentinel)
            if ((sentinelVal = checkSentinel(*currExt)) == 0) {
				// DEBUG("iterated to end of table");

			    // hit empty extent sentinel, END OF EXTS
                return NULL;
            }
			// all non-sentinels will be returned w/ -1,
			// if sentinel matches secondary or tertiary, proceed to following loop
			// for tertiary then secondary traversal
			if (sentinelVal != -1) {
				// DEBUG("proceeding to secondary/tertiary iterating");
				*extentIndex = NUM_DIR_EXTENTS;
				continue;
			}

			// SKIP next extent if empty/freed (fragmentation check)
			if (currExt->length == 0) {
				// DEBUG("skipping freed extent (length 0)");
				continue;
			}
			
			// if no conditions above are satisfied, we execute the following lines
			// return the next extent still in the primary extent array
			DEBUG("returning primary ext: location %d, length %d",
				currExt->location, currExt->length);
            return currExt;
        }
        // tertiary --> secondary traversal starts here
		// NOTE: LOADING ON DEMAND 
		// different fds from b_read() and b_write() will be calling this; can't cache each

		int pos = index - NUM_DIR_EXTENTS; 
		// DEBUG("current position in \"unwrapped\" ext array: %d", pos);

		int totIterated = 0;  // total secondary exts iterated over since first tertiary ext

		int tertSlot = 0; 	  // index in tertiary table
		int secLoc = 0; 	  // set by tertiary[tertSlot].location
		int secExt = 0; 	  // position in secondary table 

		// EXPLORING TERTIARY EXTENT TABLE --------------------------------------------------
		while (true) {
			// if ext array ONLY has one secondary table, this part will only loop once 
			// read in the block containing tertiary table ...
			extent *tertBuf = malloc(blockSize);
			if (tertBuf== NULL) {
				DEBUG_ERR("malloc failed turd buffer");
				return NULL;
			}

			if (LBAread(tertBuf, 1, exts[NUM_DIR_EXTENTS - 1].location + tertSlot) != 1) {
				DEBUG_ERR("LBAread failed for turd block # %d", tertSlot);
				free(tertBuf);
				return NULL;
			}

			extent * tertStart = &tertBuf[tertSlot];

			if (checkSentinel(*tertStart) == 0) {
				// DEBUG("reached EOE < end of exts :-) >");
				return NULL;
			}
			
			// stop reading through tertiary if we reach extentIndex
			if (pos < totIterated + tertStart->length) {
				// enter secondary table from tertiary index
				secLoc = tertStart->location; 
				// offset in secondary for corresponding ext to return
				secExt = pos - totIterated;  

				// updated variables to be used in traversing secondary extents below
				break;
			}

			// add secondary table's ext size to total exts iterated over 
			totIterated += tertStart->length; 
			// iterate to next tertiary extent (index)
			tertSlot++; 

			// EXPLORING SECONDARY EXTENT TABLE ------------------------------------------------
			// read corresponding secondary table from memory and copy first ext 
			extent * secBuf = malloc(blockSize);
			if (secBuf == NULL) {
				DEBUG_ERR("malloc failed turd buffer");
				return NULL;
			}

			if (LBAread(secBuf, 1, secLoc + secExt) != 1) {
				DEBUG_ERR("LBAread failed for turd block # %d", tertSlot);
				free(secBuf);
				return NULL;
			}

			extent * currExt = &secBuf[secExt]; 
			(*extentIndex)++;

			if (checkSentinel(*currExt) == 0) {
				// DEBUG("iterated to end of secondary");
				return NULL;
			}

			// SKIP next extent if empty/freed (fragmentation check)
			if (currExt->length == 0) {
				// DEBUG("skipping freed extent (length 0)");
				continue;
			}

			DEBUG("returning valid secondary ext, location: %d, length %d", 
			currExt->location, currExt->length);
			return currExt;
		}
    }
}

// checks current extent array (be it primary, secondary, or tertiary) 
// and returns first instance of sentinel for an empty extent
// NOT a freed extent (table to be cleaned up by a separate function)
int findFirstFree(const extent * exts) {
	int i = 0;
	int sentinel;

	while(true) {
		if ((sentinel = checkSentinel(exts[i])) == 0) {
			// DEBUG("returning at %d index, empty extent at location %d", i, exts[i].location);
			return i;
		}
		if (sentinel == -2 | sentinel == -3) {
			// this is the last ext in table, returning size of table!
			return i;
		}
		i++;	
    }
	// currently operates under the assumption there will always be an empty ext....
	// needs to track size of secondary/tertiary 
    return -1;
}

// checks passed extent if the length contains our sentinel value 
// for either secondary or tertiary table locations
// this is called to determine which case to use in growFile() and getNextExtent()
int checkSentinel(extent ext) { 
	switch (ext.length) {
		case 0:
			if (ext.location == 0) {
				//DEBUG("returning empty ext sentinel");
				return 0;
			}

		case SECONDARY_EXT_SENTINEL:
			//DEBUG("returning secondary sentinel value");
        	return -2; 

    	case TERTIARY_EXT_SENTINEL:
			//DEBUG("returning tertiary sentinel value");
			return -3;	

		default:
			//DEBUG("passed ext is not a sentinel");
			return -1;
    }
}
