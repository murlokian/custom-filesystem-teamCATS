/**************************************************************
* Class::  CSC-415-0# Fall 2025
* Name::
* Student IDs::
* GitHub-Name::
* Group-Name::
* Project:: Basic File System
*
* File:: directory.c
*
* Description::
*
**************************************************************/

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

#include "fsDir.h"
#include "mfs.h"
#include "fsFreeSpace.h"
#include "fsLow.h"

directory_entry *createDirectory(int initialEntryCount, directory_entry *parent, uint64_t blockSize)
{
    // calculate requested memory depending on requested entries
    int memoryNeeded = initialEntryCount * sizeof(directory_entry);

    // disK I/O block units
    int blocksNeeded = (memoryNeeded + (blockSize - 1)) / blockSize;
    memoryNeeded = blocksNeeded * blockSize;

    // recalc how much entries fit in allocated space
    int entryCount = memoryNeeded / sizeof(directory_entry);

    // allocate RAM 
    directory_entry *newDir = malloc(memoryNeeded);
    if (newDir == NULL)
    {
        return NULL;
    }

    // unused entries begin in an empty state
    memset(newDir, 0, memoryNeeded);

    // get blocks on disk for this directory
    int location = getBlocks(blocksNeeded); // getBlocks to be written in fsFreeSpace.c
    if (location < 0)
    {
        free(newDir);
        return NULL;
    }

    // timestamp for directory metadata
    time_t curDate = time(NULL);

    // "." entry points
    strcpy(newDir[0].name, ".");
    newDir[0].entryType = true;
    newDir[0].size = entryCount * sizeof(directory_entry);
    newDir[0].creationDate = curDate;
    newDir[0].lastEditDate = curDate;
    newDir[0].lastAccessDate = curDate;
    newDir[0].exts[0].location = location;
    newDir[0].exts[0].length = blocksNeeded * blockSize;

    // if parent NULL this is root directory
    // ".." should point back to the root itself
    if (parent == NULL)
    {
        parent = newDir;
    }

    // ".." entry points to parent directoty
    strcpy(newDir[1].name, "..");
    newDir[1].entryType = true;
    newDir[1].size = parent[0].size;
    newDir[1].creationDate = parent[0].creationDate;
    newDir[1].lastEditDate = parent[0].lastEditDate;
    newDir[1].lastAccessDate = parent[0].lastAccessDate;
    newDir[1].exts[0].location = parent[0].exts[0].location;
    newDir[1].exts[0].length = parent[0].exts[0].length;

    // all remaining unused entries 
    // name[0] = '\0' for free directory slot
    for (int i = 2; i < entryCount; i++)
    {
        newDir[i].name[0] = '\0';
    }

    // write entire directory to disk
    if (LBAwrite(newDir, blocksNeeded, location) != blocksNeeded)
    {
        free(newDir);
        return NULL;
    }

    return newDir;
}

int initRootDirectory(uint64_t blockSize)
{
    // idk try 32 entries for now
    directory_entry *root = createDirectory(32, NULL, blockSize);
    if (root == NULL)
    {
        return -1;
    }

    // root directory "." entry
    int rootLocation = root[0].exts[0].location;

    // free memory ofc 
    free(root);
    return rootLocation;
}