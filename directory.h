/**************************************************************
* Class::  CSC-415-0# Fall 2025
* Name::
* Student IDs::
* GitHub-Name::
* Group-Name::
* Project:: Basic File System
*
* File:: directory.h
*
* Description::
*
**************************************************************/

#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <stdint.h>
#include "mfs.h"

// create directory and allocate disk space
directory_entry *createDirectory(int initialEntryCount, directory_entry *parent, uint64_t blockSize);

// create root directory and return starting block location
int initRootDirectory(uint64_t blockSize);

#endif