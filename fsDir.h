/**************************************************************
* Class::  CSC-415-03 Spring 2026
* Name::  Allie Young, Patricia Ann Payumo
* Student IDs::  923129607, 923555591
* GitHub-Name::  milo-pa
* Group-Name::  CATS
* Project:: Basic File System
*
* File:: fsDir.h
*
* Description:: This is a header file for the extent and directory entry structs
* and related functions.  
*
**************************************************************/

#ifndef FSDIR_H
#define FSDIR_H

#include <stdint.h>
#include <stdbool.h>

// MAX_ENTRY_NAME_SIZE shall be defined as 255 in order to provide a reasonable
// file name length that is similar to other major operating systems but fills in 
// all padding
#define MAX_ENTRY_NAME_SIZE 255
#define NUM_DIR_EXTENTS       5

typedef struct extent {  // 16 bytes
	
    // location is an unsigned 64-bit value which is the location of a starting
    // block of contiguous free space. 0 signifies invalid/unused extent  
    uint64_t location;
  
    // length is an unsigned 64-bit value which contains the total amount of blocks
    // that this extent spans
    uint64_t length;

} extent;

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

// create directory and allocate disk space
directory_entry *createDirectory(int initialEntryCount, directory_entry *parent);

#endif