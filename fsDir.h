/*************************************************************
* Class::  CSC-415-03 Spring 2026
* Name::  Allie Young, Patricia Ann Payumo, Rosa Melton, Milo Pesce Ares
* Student IDs::  923129607, 923555591, 921337778, 921296821
* GitHub-Name::  milo-pa
* Group-Name::  CATS
* Project:: Basic File System
*
* File:: fsDir.h
*
* Description:: This is a header file for the extent and directory 
* entry structs and individual directory funcs
*
**************************************************************/

#ifndef FSDIR_H
#define FSDIR_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#include "fsStorage.h"

// MAX_ENTRY_NAME_SIZE shall be defined as 255 in order to provide a reasonable
// file name length that is similar to other major operating systems but fills in 
// all padding
#define MAX_ENTRY_NAME_SIZE 255

// NUM_DIR_EXTENTS is the number of extents that a directory entry can have.
// 5 should be sufficient for most files/directories, but secondary extent tables 
// can be added if needed.
#define NUM_DIR_EXTENTS 5

#define DEFAULT_NUM_DIR_ENTRIES 50

typedef struct directory_entry {

	// size member is an unsigned 64-bit value which is the size of the entry
	// measured in bytes
	uint64_t size;

	// creationDate, lastEditDate, and lastAccessDate are all time_t (8 byte)
	// that represents the number of seconds since some common epoch (tbd)
	time_t creationDate;
	time_t lastEditDate;
	time_t lastAccessDate;

	// exts is an array of extents for indexing each entry's data in memory  
	extent exts[NUM_DIR_EXTENTS];

	// entryType member is a boolean that represents whether an entry is
	// a directory or a file. 
	// 0 for file, 1 for directory
	bool entryType;

	// name member is a char array that contains the name of the entry
	// Names are unique within the scope of a directory
	// Max size of name is defined by a macro
	// if value is "\0" then the entry is unused
	char name[MAX_ENTRY_NAME_SIZE];

} directory_entry;

// Global instance of root directory so it can be accessed across many functions and files
// and because there is only one root directory per volume.
extern directory_entry* rootGlobal;

// Global instance of current working directory so it can be accessed across many functions
// and files and because there is only one current working directory.
extern directory_entry* cwdGlobal;

// Populates and writes a directory to disk. Minimum 2 initial entries for "." and ".." required
// Pass NULL for parent if this is the root directory.
// Returns pointer to directory_entry struct with allocated memory rounded up to blockSize
directory_entry * createDirectory(int initialEntryCount, directory_entry *parent);

// Writes the given directory to disk
// Returns 0 on success, -1 on failure
int writeDirectory(directory_entry *dir);

// Takes in a directory entry and loads all the directory entries in that
// directory and returns a pointer to the directory entries in memory
// If passed NULL, loads the root directory  
directory_entry * loadDir(directory_entry * dir);

// Somewhat similar logic to strtok_r, helper for multiple calls of b_read(), b_write()
// function updates extentIndex as the 'current position' for the next call (acting as our saveptr)
// iterate from beginning of DE's extent array up to extentIndex and returns updated position
// Updated position to be used when calling LBAread() or LBAwrite()
// TODO - move to helper file
extent * getNextExtent(extent * exts, int * extentIndex);

// Expands the given file by the given number of blocks and updates the directory 
// entry accordingly. Will create secondary extent tables as needed if the number 
// of extents exceeds NUM_DIR_EXTENTS.
// Returns 0 on success, -1 on failure
int growFile(directory_entry * file, int additionalBlocks);

typedef int dir_index;

// Struct to hold information about the result of parsing a path. This is used to return
// multiple values from the parsePath function.
// entryIndex of -1 indicates nonexistent last element in cases like mkdir  
// entryIndex of -2 indicates path given was "/"  
typedef struct parsePathInfo
{
	directory_entry *parentDir;
	dir_index entryIndex;
	char *lastElementName;
} parsePathInfo;

// Helper function to free in memory fields in the parsePathInfo struct  
void freePPI(parsePathInfo* ppi);

// Parses the given path and passes back information about the path in the
// parsePathInfo struct.
// ppi.entryIndex of -1 indicates nonexistent last element in cases like mkdir  
// ppi.entryIndex of -2 indicates path given was "/"  
// Caller must call freePPI() before cwdGlobal is changed  
// Returns 0 on success, -1 on failure
int parsePath(const char* path, parsePathInfo* ppi);

// Finds the index of the directory entry with the given name in the given directory.
// Returns the index of the entry if found, -1 if not found.
dir_index findDirEntry(directory_entry *dir, const char *name);

// Finds the index of the first unused directory entry in the given directory.
// If no unused entry is found, it will expand directory as needed and return the
// index of the first entry in the newly allocated space.
dir_index findUnusedDirEntry(directory_entry *dir);

#endif