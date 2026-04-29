/**************************************************************
 * Class::  CSC-415-03 Spring 2026
 * Name:: Rosa Melton, Patricia Ann Payumo, Abha Deshpande
 * Student IDs:: 921337778, 923555591, 923314454
 * GitHub-Name:: milo-pa
 * Group-Name:: CATS
 * Project:: Basic File System
 *
 * File:: b_io.c
 *
 * Description:: Basic File System - Key File I/O Operations
 *
 **************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h> // for malloc
#include <string.h> // for memcpy
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "b_io.h"
#include "fsLow.h"
#include "fsDir.h"
#include "fsVCB.h" // used to get blockSize (can fix/change; current shortcut...)
#include "debug.h"

#define MAXFCBS 20

typedef struct b_fcb
{
	/** TODO add al the information you need in the file control block **/
	// TWO MORE VARIABLES TO ADD BASED ON BIERMAN'S ASGN5 APPROACH
	directory_entry *de; // holds the low level systems file info

	char *buf;	   // holds the open file buffer
	int fileIndex; // holds the current byte position in the file
	int buflen;	   // holds how many valid bytes are in the buffer
	int flags;     // stores how the file was opened
} b_fcb;

b_fcb fcbArray[MAXFCBS];

int startup = 0; // Indicates that this has not been initialized

// Method to initialize our file system
void b_init()
{
	// init fcbArray to all free
	for (int i = 0; i < MAXFCBS; i++)
	{
		fcbArray[i].buf = NULL; // indicates a free fcbArray
	}

	startup = 1;
}

// Method to get a free FCB element
b_io_fd b_getFCB()
{
	for (int i = 0; i < MAXFCBS; i++)
	{
		if (fcbArray[i].buf == NULL)
		{
			return i; // Not thread safe (But do not worry about it for this assignment)
		}
	}
	return (-1); // all in use
}

// Interface to open a buffered file
// Modification of interface for this assignment, flags match the Linux flags for open
// O_CREAT, O_TRUNC, O_APPEND, O_RDONLY, O_WRONLY, or O_RDWR
b_io_fd b_open(char *filename, int flags)
{
	b_io_fd returnFd;

	//*** TODO ***:  Modify to save or set any information needed
	//
	//

	if (startup == 0)
		b_init(); // Initialize our system

	returnFd = b_getFCB(); // get our own file descriptor
						   // check for error - all used FCB's

	if (returnFd == -1)
	{
		DEBUG("Error: No free file control blocks");
		return -1;
	}

	fcbArray[returnFd].flags = flags;  //storing flags for b_read

	return (returnFd); // all set
}

// Interface to seek function
int b_seek(b_io_fd fd, off_t offset, int whence)
{
	if (startup == 0)
		b_init(); // Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
	{
		return (-1); // invalid file descriptor
	}

	// make sure FCB is in use
	if (fcbArray[fd].de == NULL)
	{
		DEBUG("ERROR: FD not in use");
		return -1;
	}

	int newPos = 0;

	// calculate new file pos
	switch (whence)
	{
	case SEEK_SET:
		newPos = offset; // set pos to exact offset
		break;

	case SEEK_CUR:
		newPos = fcbArray[fd].fileIndex + offset; // move relative to current pos
		break;

	case SEEK_END:
		newPos = fcbArray[fd].de->size + offset; // move relative to eof
		break;

	default:
		DEBUG("ERROR: invalid whence value"); // invalid whence value
		return -1;
	}

	if (newPos < 0)
	{ // prevent negative file pos
		DEBUG("Error: Seek attempted before beginning of file");
		return -1;
	}

	int blockSize = VCBGlobal->blockSize;

	// figure out which block the old and new pos are
	int oldBlock = fcbArray[fd].fileIndex / blockSize;
	int newBlock = newPos / blockSize;

	// invalidate buffer if moved to a diff block
	if (oldBlock != newBlock)
	{
		// clear valid data in buff
		fcbArray[fd].buflen = 0;
	}

	// update file pos
	fcbArray[fd].fileIndex = newPos;

	return newPos; // return new pos
}

// Interface to write function
int b_write(b_io_fd fd, char *buffer, int count)
{
	if (startup == 0)
		b_init(); // Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
	{
		return (-1); // invalid file descriptor
	}

	return (0); // Change this
}

// Interface to read a buffer

// Filling the callers request is broken into three parts
// Part 1 is what can be filled from the current buffer, which may or may not be enough
// Part 2 is after using what was left in our buffer there is still 1 or more block
//        size chunks needed to fill the callers request.  This represents the number of
//        bytes in multiples of the blocksize.
// Part 3 is a value less than blocksize which is what remains to copy to the callers buffer
//        after fulfilling part 1 and part 2.  This would always be filled from a refill
//        of our buffer.
//  +-------------+------------------------------------------------+--------+
//  |             |                                                |        |
//  | filled from |  filled direct in multiples of the block size  | filled |
//  | existing    |                                                | from   |
//  | buffer      |                                                |refilled|
//  |             |                                                | buffer |
//  |             |                                                |        |
//  | Part1       |  Part 2                                        | Part3  |
//  +-------------+------------------------------------------------+--------+
int b_read(b_io_fd fd, char *buffer, int count)
{

	if (startup == 0)
		b_init(); // Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
	{
		return (-1); // invalid file descriptor
	}

	// helper getNextExtent() called to handle iterating through a directory entry's extents
	// this is so this caller doesn't have to worry about current block/position in
	// entry as b_read() is called
	// getNextExtent handles if we need to traverse secondary/tertiary tables for this file
	int *extIndex; // equivalent of saveptr in strtok_r()
	extent *currExt = getNextExtent(fcbArray[fd].de->exts, extIndex);

	int currPos = currExt->length * VCBGlobal->blockSize;

	while ((currExt = getNextExtent(fcbArray[fd].de->exts, extIndex)) != NULL)
	{
		LBAread(buffer + currPos, currExt->length, currExt->location);

		currPos = currExt->length * VCBGlobal->blockSize;
	}

	return (0); // Change this
}

// Interface to Close the file
int b_close(b_io_fd fd)
{

	// Ensure that the system has been initialized before accessing the file control blocks
	if (startup == 0)
	{
		DEBUG("Error: System not initialized");
		return -1;
	}

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
	{
		DEBUG("Error: Invalid file descriptor");
		return (-1);
	}

	// Check that the provided file descriptor is actually in use
	if (fcbArray[fd].de == NULL)
	{
		DEBUG("Error: File not open for this descriptor");
		return -1;
	}

	fcbArray[fd].de = NULL; // Mark this FCB as free again

	return 0;
}
