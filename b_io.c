/**************************************************************
 * Class::  CSC-415-03 Spring 2026
 * Name:: Rosa Melton, Patricia Ann Payumo, Abha Deshpande,
 * 			Milo Pesce Ares, Allie Young
 * Student IDs:: 921337778, 923555591, 923314454, 
 * 			9212926821, 923129607
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
#include "fsVCB.h"
#include "fsStorage.h"
#include "debug.h"

#define MAXFCBS 20

// Struct for storing current place in file
typedef struct
{
	int extentIndex; // which extent we're on
	int blockOffset; // how many blocks into the current extent we are
	extent *curr;	 // the current extent
} extent_cursor;

typedef struct b_fcb
{
	/** TODO add al the information you need in the file control block **/
	// TWO MORE VARIABLES TO ADD BASED ON BIERMAN'S ASGN5 APPROACH
	directory_entry *de; // holds the low level systems file info
	directory_entry *parent; // for writing metadata changes
	int entryIndex; // the index of the directory inside its parent  

	char *buf;	   // holds the open file buffer
	int fileIndex; // holds the current byte position in the file
	int buflen;	   // holds how many valid bytes are in the buffer
	int flags;     // stores how the file was opened

	extent_cursor cursor;
} b_fcb;


b_fcb fcbArray[MAXFCBS];

int startup = 0; // Indicates that this has not been initialized

static uint64_t *getNextBlock(directory_entry *de, extent_cursor *cur, uint64_t *out);
static int seekCursor(directory_entry *de, extent_cursor *cur, int targetBlock);

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

	if (startup == 0)
		b_init(); // Initialize our system

	returnFd = b_getFCB(); // get our own file descriptor
						   // check for error - all used FCB's
	if (returnFd == -1)
	{
		DEBUG("Error: No free file control blocks");
		return -1;
	}
	DEBUG("File control block found with index %d", returnFd);

	// Get info about path  
	parsePathInfo ppi;
	if(parsePath(filename, &ppi) != 0) {
		DEBUG("parsePath failed");
		freePPI(&ppi);
		return -1;
	}

	// Nonexistent last element  
	if(ppi.entryIndex == -1) {
		DEBUG("Nonexistent last element");

		// Exit if not creating file  
		if(!(flags & O_CREAT)) {
			DEBUG("Create flag not on");
			freePPI(&ppi);
			return -1;
		}
		// Handle CREATE  

		// Null check  
		if(ppi.parentDir == NULL) {
			freePPI(&ppi);
			return -1;
		}

		// Find unused entry in parent  
		int emptyEntry = findUnusedDirEntry(ppi.parentDir);
		if(emptyEntry == -1) {
			DEBUG("No empty entries");
			freePPI(&ppi);
			return -1;
		}

		// Fill in entry for new file  
		memset(&ppi.parentDir[emptyEntry], 0, sizeof(directory_entry));
		ppi.parentDir[emptyEntry].size = 0;
		time_t now = time(NULL);
		ppi.parentDir[emptyEntry].creationDate = now;
		ppi.parentDir[emptyEntry].lastEditDate = now;
		ppi.parentDir[emptyEntry].lastAccessDate = now;
		ppi.parentDir[emptyEntry].entryType = 0;
		strncpy(ppi.parentDir[emptyEntry].name, ppi.lastElementName, MAX_ENTRY_NAME_SIZE - 1);
		ppi.parentDir[emptyEntry].name[MAX_ENTRY_NAME_SIZE - 1] = '\0';
		DEBUG("Created new file entry with name: %s", ppi.parentDir[emptyEntry].name);

		// Update parent metadata  
		ppi.parentDir[0].lastEditDate = now;

		ppi.entryIndex = emptyEntry;
	} else {
		// Null check  
		if(ppi.parentDir == NULL) {
			freePPI(&ppi);
			return -1;
		}

		// Target is root  
		if(ppi.entryIndex == -2) {
			DEBUG("Called on root, not a file");
			freePPI(&ppi);
			return -1;
		}

		// Target is a directory  
		if(ppi.parentDir[ppi.entryIndex].entryType == 1) {
			DEBUG("Called on directory, not file");
			freePPI(&ppi);
			return -1;
		}
	}

	// Handle TRUNCATE  
	if(flags & O_TRUNC) {
		// Exit if read only file  
		if((flags & O_ACCMODE) == O_RDONLY) {
			DEBUG("Truncate called on read only file");
			freePPI(&ppi);
			return -1;
		}

		// Free blocks associated with file  
		if(freeBlocks(ppi.parentDir[ppi.entryIndex].exts) != 0) {
			DEBUG("freeBlocks failed during truncate");
			freePPI(&ppi);
			return -1;
		}

		// Update file data  
		ppi.parentDir[ppi.entryIndex].size = 0;
		time_t now = time(NULL);
		ppi.parentDir[ppi.entryIndex].lastEditDate = now;
		ppi.parentDir[ppi.entryIndex].lastAccessDate = now;
		for(int i = 0; i < NUM_DIR_EXTENTS; i++) {
			ppi.parentDir[ppi.entryIndex].exts[i].location = 0;
			ppi.parentDir[ppi.entryIndex].exts[i].length = 0;
		}
		DEBUG("Truncated file %s", ppi.parentDir[ppi.entryIndex].name);

		// Update parent metadata  
		ppi.parentDir[0].lastEditDate = now;
	}

	// Always update access date  
	ppi.parentDir[ppi.entryIndex].lastAccessDate = time(NULL);
	if(writeDirectory(ppi.parentDir) != 0) {
		DEBUG("writeDirectory failed");
		freePPI(&ppi);
		return -1;
	}

	// Allocate memory for buffer  
	char* buffer = malloc(VCBGlobal->blockSize);
	if(buffer == NULL) {
		DEBUG("malloc failed");
		freePPI(&ppi);
		return -1;
	}

	// Set fields in file control block  
	fcbArray[returnFd].de = &ppi.parentDir[ppi.entryIndex];
	fcbArray[returnFd].parent = ppi.parentDir;
	fcbArray[returnFd].entryIndex = ppi.entryIndex;
	fcbArray[returnFd].buf = buffer;
	fcbArray[returnFd].buflen = 0;
	fcbArray[returnFd].flags = flags;
	fcbArray[returnFd].cursor = (extent_cursor){0};

	// Move index to end if APPENDing  
	if(flags & O_APPEND) {
		fcbArray[returnFd].fileIndex = fcbArray[returnFd].de->size;
		DEBUG("Opened for appending, file index moved to %d", fcbArray[returnFd].fileIndex);

		int lastBlock = fcbArray[returnFd].de->size / VCBGlobal->blockSize;
		int partialBytes = fcbArray[returnFd].de->size % VCBGlobal->blockSize;

		if (seekCursor(fcbArray[returnFd].de, &fcbArray[returnFd].cursor, lastBlock) != 0)
		{
			DEBUG_ERR("seekCursor failed for O_APPEND");
			free(buffer);
			freePPI(&ppi);
			return -1;
		}

		// If the file doesn't end on a block boundary, read the partial block
		// into the fcb buffer so b_write appends correctly
		if (partialBytes > 0)
		{
			uint64_t lba;
			if (getNextBlock(fcbArray[returnFd].de, &fcbArray[returnFd].cursor, &lba) == NULL)
			{
				DEBUG_ERR("getNextBlock failed reading partial block for O_APPEND");
				free(buffer);
				freePPI(&ppi);
				return -1;
			}

			LBAread(buffer, 1, lba);
			fcbArray[returnFd].buflen = partialBytes;
			DEBUG("read partial last block from LBA %lu, buflen set to %d", lba, partialBytes);
		}
	} else {
		fcbArray[returnFd].fileIndex = 0;
	}

	// Clean up used resources  
	if(ppi.lastElementName != NULL) {
		free(ppi.lastElementName);
		ppi.lastElementName = NULL;
	}
	ppi.parentDir = NULL;

	return returnFd;
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

int b_write(b_io_fd fd, char *buffer, int count)
{
	if (startup == 0)
		b_init();

	if ((fd < 0) || (fd >= MAXFCBS))
	{
		DEBUG("invalid file descriptor %d", fd);
		return -1;
	}

	b_fcb *fcb = &fcbArray[fd];

	if (!(fcb->flags & O_WRONLY) && !(fcb->flags & O_RDWR))
	{
		DEBUG("fd %d not open for writing, flags: %d", fd, fcb->flags);
		return -1;
	}

	int blockSize = VCBGlobal->blockSize;
	int bytesWritten = 0;
	int bufIndex = 0;

	DEBUG("fd %d: writing %d bytes (fileIndex %d, buflen %d)",
		  fd, count, fcb->fileIndex, fcb->buflen);

	while (count > 0)
	{
		int spaceInBuf = blockSize - fcb->buflen;

		// Optimization: buffer is empty and caller has a full block or more.
		//  Write directly from the caller's buffer — no copy needed.
		if (fcb->buflen == 0 && count >= blockSize)
		{
			uint64_t lba;
			if (getNextBlock(fcb->de, &fcb->cursor, &lba) == NULL)
			{
				DEBUG("out of extents on fast path, %d bytes remaining", count);

				if (growFile(fcb->de, 100) == -1) {
					DEBUG("failed to allocate blocks");
				}

				if (getNextBlock(fcb->de, &fcb->cursor, &lba) == NULL)
				{
					DEBUG("getNextBlock failed after allocation on fast path");
					break;
				}

				break;
			}

			DEBUG("fast path: writing block directly to LBA %lu", lba);
			LBAwrite(buffer + bufIndex, 1, lba);

			bufIndex += blockSize;
			count -= blockSize;
			fcb->fileIndex += blockSize;
			bytesWritten += blockSize;

			if ((uint64_t)fcb->fileIndex > fcb->de->size)
				fcb->de->size = (uint64_t)fcb->fileIndex;

			continue;
		}

		// copy as much as fits into the fcb buffer, then flush if the buffer is now full
		int toCopy = (count < spaceInBuf) ? count : spaceInBuf;

		memcpy(fcb->buf + fcb->buflen, buffer + bufIndex, toCopy);

		fcb->buflen += toCopy;
		bufIndex += toCopy;
		count -= toCopy;
		fcb->fileIndex += toCopy;
		bytesWritten += toCopy;

		if ((uint64_t)fcb->fileIndex > fcb->de->size)
			fcb->de->size = (uint64_t)fcb->fileIndex;

		if (fcb->buflen == blockSize)
		{
			uint64_t lba;
			if (getNextBlock(fcb->de, &fcb->cursor, &lba) == NULL)
			{

				DEBUG("out of extents on normal path, %d bytes remaining", count);

				if (growFile(fcb->de, 100) == -1)
				{
					DEBUG("failed to allocate blocks");
				}

				if (getNextBlock(fcb->de, &fcb->cursor, &lba) == NULL)
				{
					DEBUG("getNextBlock failed after allocation on fast path");
					break;
				}

				break;
			}

			DEBUG("normal path: flushing full block to LBA %lu", lba);
			LBAwrite(fcb->buf, 1, lba);
			fcb->buflen = 0;
		}
	}

	DEBUG("fd %d: wrote %d bytes, fileIndex now %d, buflen %d",
		  fd, bytesWritten, fcb->fileIndex, fcb->buflen);

	return bytesWritten;
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
	int extIndex = 0; // equivalent of saveptr in strtok_r()
	
    //flag check
    if (fcbArray[fd].flags & O_WRONLY)
        return -1;
	
	
	// cap count to remaining bytes in file
	int remainingBytes = fcbArray[fd].de->size - fcbArray[fd].fileIndex;
	if(count > remainingBytes) count = remainingBytes;
	if(count == 0) return 0;
	int totalBytesCopied = 0;
	int blockSize = VCBGlobal -> blockSize;
	//Part 1 -- copy from existing buffer 

	int bytesAvailable = fcbArray[fd].buflen - (fcbArray[fd].fileIndex % blockSize);
	int part1 = (bytesAvailable < count)? bytesAvailable : count;
	memcpy(buffer, fcbArray[fd].buf + (fcbArray[fd].fileIndex % blockSize), part1);

	totalBytesCopied += part1;
	fcbArray[fd].fileIndex += part1;
	count -= part1;

	//if count is already 0 
	if(count == 0) return totalBytesCopied;


	//Part 2 -- read full blocks directly 

	while(count >= blockSize){
		extent *curExt = getNextExtent(fcbArray[fd].de -> exts,&extIndex );
		if (curExt == NULL){
			DEBUG("getNextExtent returned null in b_read: b_io.c");
			return totalBytesCopied;
		}
		LBAread(buffer + totalBytesCopied, 1, curExt->location);
		count -= blockSize;
		totalBytesCopied += blockSize;
		fcbArray[fd].fileIndex += blockSize;

	}
	//Part 3 -- read remaining partial block 
	//get the next extent 
	extent *curExt = getNextExtent(fcbArray[fd].de ->exts, &extIndex);
	if(curExt == NULL){
		DEBUG("Failed to get extent in b_read(): b_io.c ");
		return -1;
	}
	//read one block into buf
	LBAread(fcbArray[fd].buf, 1, curExt->location);
	//copy count bytes from buf to buffer
	memcpy(buffer + totalBytesCopied, fcbArray[fd].buf, count);
	//update buflen, fileIndex and totalBytesCopied
	fcbArray[fd].buflen = blockSize;
	fcbArray[fd].fileIndex += count;
	totalBytesCopied += count;

	return totalBytesCopied;
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

	// Flush any remaining partial block in the buffer
	if (fcbArray[fd].buflen > 0)
	{
		uint64_t lba;
		if (getNextBlock(fcbArray[fd].de, &fcbArray[fd].cursor, &lba) == NULL)
		{
			DEBUG("out of extents flushing partial block on close, %d bytes lost", fcbArray[fd].buflen);
		}
		else
		{
			DEBUG("flushing partial block of %d bytes to LBA %lu on close", fcbArray[fd].buflen, lba);
			LBAwrite(fcbArray[fd].buf, 1, lba);
			fcbArray[fd].buflen = 0;
		}
	}

	// Update access timestamp
	time_t now = time(NULL);
	fcbArray[fd].de->lastAccessDate = now;

	// Update write timestamp only if write flag enabled
	if ((fcbArray[fd].flags & O_WRONLY) || (fcbArray[fd].flags & O_RDWR))
		fcbArray[fd].de->lastEditDate = now;

	if(fcbArray[fd].parent != NULL) {
		if(writeDirectory(fcbArray[fd].parent) != 0) {
			DEBUG("writeDirectory failed");
			return -1;
		}
	}

	// Free used resources  
	if(fcbArray[fd].buf != NULL) {
		free(fcbArray[fd].buf);
		fcbArray[fd].buf = NULL;
	}

	if(fcbArray[fd].parent != NULL && 
		fcbArray[fd].parent != rootGlobal &&
		fcbArray[fd].parent != cwdGlobal) {
		free(fcbArray[fd].parent);
	}

	// Mark this FCB as free again, reset fields  
	fcbArray[fd].de = NULL;
	fcbArray[fd].parent = NULL;
	fcbArray[fd].entryIndex = -1;
	fcbArray[fd].fileIndex = 0;
	fcbArray[fd].buflen = 0;
	fcbArray[fd].flags = 0;
	fcbArray[fd].cursor = (extent_cursor){0};
	DEBUG("Closed file at fcb index %d", fd);

	return 0;
}

// Walks extents to position cursor at the given file-block index.
// Called once from b_open for O_APPEND. Returns 0 on success, -1 on failure.
static int seekCursor(directory_entry *de, extent_cursor *cur, int targetBlock)
{
	int accumulated = 0;

	while (1)
	{
		extent *ext = getNextExtent(de->exts, &cur->extentIndex);
		if (ext == NULL)
		{
			DEBUG_ERR("ran out of extents seeking to block %d", targetBlock);
			return -1;
		}

		if (accumulated + (int)ext->length > targetBlock)
		{
			cur->curr = ext;
			cur->blockOffset = targetBlock - accumulated;
			DEBUG("cursor seeked to block %d: LBA %u, offset %d",
				  targetBlock, cur->curr->location, cur->blockOffset);
			return 0;
		}

		accumulated += (int)ext->length;
		free(ext);
	}
}

// Helper function that wraps getNextExtent(). This is called by b_write()
static uint64_t *getNextBlock(
	directory_entry *de, extent_cursor *cur, uint64_t *out)
{
	/* Advance to next extent if current is exhausted */
	if (cur->curr == NULL || cur->blockOffset >= (int)cur->curr->length)
	{
		if (cur->curr != NULL)
		{
			DEBUG("extent at LBA %u exhausted, fetching next", cur->curr->location);
			free(cur->curr);
		}

		cur->curr = getNextExtent(de->exts, &cur->extentIndex);
		if (cur->curr == NULL)
		{
			DEBUG("no more extents available");
			return NULL;
		}

		cur->blockOffset = 0;
	}

	*out = (uint64_t)(cur->curr->location + cur->blockOffset);
	cur->blockOffset++;
	return out;
}
