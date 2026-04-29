/**************************************************************
 * Class::  CSC-415-03 Spring 2026
 * Name:: Patricia Ann Payumo, Abha Deshpande
 * Student IDs:: 923555591, 923314454
 * GitHub-Name::  milo-pa
 * Group-Name::  CATS
 * Project:: Basic File System
 *
 * File:: mfsGod.c
 *
 * Description:: Implements mfs.h functions for creating and
 *  deleting files and directories
 *
 **************************************************************/

#include <stdlib.h>
#include <string.h>

#include "mfs.h"
#include "fsDir.h"
#include "debug.h"
#include "fsStorage.h"
#include <time.h>

int fs_mkdir(const char *pathname, mode_t mode)
{

    parsePathInfo ppi; // this is where parsepath returns a struct to mkdir

    // called considering parsePath(char *pathname, parsedPath *s) as prototype
    int result = parsePath(pathname, &ppi);
    if (result == -1)
    {
        DEBUG("Parse Path failed: mfs.c");
        return -1;
    }

    // Check if the directory already exists
    if (ppi.entryIndex != -1)
    {
        DEBUG("Directory entry already exists. Mkdir failed: mfs.h");
        freePPI(&ppi);
        return -1;
    }

    // Check if parent exists and isDir
    if (ppi.parentDir == NULL)
    {
        freePPI(&ppi);
        return -1;
    }

    // call createDir(10, s.parent) -- this allocates disk space
    directory_entry *newDir = createDirectory(DEFAULT_NUM_DIR_ENTRIES, ppi.parentDir);

    // check if the directory was actually created
    if (newDir == NULL) {
        freePPI(&ppi);
        return -1;
    }

    int index = findUnusedDirEntry(ppi.parentDir);
    // if empty, what about the directory createDirectory created?
    if (index == -1)
    {
        free(newDir);
        newDir = NULL;
        freePPI(&ppi);
        return -1;
    }

    // fill the slot
    ppi.parentDir[index] = newDir[0];
    strcpy(ppi.parentDir[index].name, ppi.lastElementName);

    //update parent 
    time_t curDate = time(NULL);
    ppi.parentDir[0].lastEditDate = curDate;
    ppi.parentDir[0].lastAccessDate = curDate;
    // write updated parent back to disk
    if(writeDirectory(ppi.parentDir) == -1) {
        free(newDir);
        newDir = NULL;
        freePPI(&ppi);
        return -1;
    }

    free(newDir);
    newDir = NULL;
    freePPI(&ppi);
    return 0;
}

// remove empty directory from file sys
// directory can only be removed if contains no entries except "." and ".."
int fs_rmdir(const char *pathname)
{
    parsePathInfo ppi;

    // find directory entry inside its parent directory
    if (parsePath(pathname, &ppi) != 0)
    {
        return -1;
    }

    // deny invalid parse results
    if (ppi.parentDir == NULL || ppi.entryIndex < 0)
    {
        freePPI(&ppi);
        return -1;
    }

    directory_entry *entry = &ppi.parentDir[ppi.entryIndex];

    // only removes directories
    if (entry->entryType != 1)
    {
        freePPI(&ppi);
        return -1;
    }

    // load directory to check if its empty
    directory_entry *dir = loadDir(entry);
    if (dir == NULL)
    {
        freePPI(&ppi);
        return -1;
    }

    int numEntries = entry->size / sizeof(directory_entry);

    // only "." and ".."
    for (int i = 2; i < numEntries; i++)
    {
        if (dir[i].name[0] != '\0')
        {
            free(dir);
            dir = NULL;
            freePPI(&ppi);
            return -1;
        }
    }

    // freeeeeeeeeeeeee directory's blocks
    if (freeBlocks(entry->exts) != 0)
    {
        free(dir);
        dir = NULL;
        freePPI(&ppi);
        return -1;
    }

    // remove this entry from parent directory
    entry->name[0] = '\0';
    entry->size = 0;

    // save updated parent directory
    if (writeDirectory(ppi.parentDir) != 0)
    {
        free(dir);
        dir = NULL;
        freePPI(&ppi);
        return -1;
    }

    free(dir);
    dir = NULL;
    freePPI(&ppi);
    return 0;
}

// remove file by freeing its blocks and clearing its directory entry
int fs_delete(char *filename)
{
    parsePathInfo ppi;

    // find file inside parent directory
    if (parsePath(filename, &ppi) != 0)
    {
        freePPI(&ppi);
        return -1;
    }

    // deny invalid parse results
    if (ppi.parentDir == NULL || ppi.entryIndex < 0)
    {
        freePPI(&ppi);
        return -1;
    }

    directory_entry *entry = &ppi.parentDir[ppi.entryIndex];

    // only removes regular files
    if (entry->entryType != 0)
    {
        freePPI(&ppi);
        return -1;
    }

    // return file's blocks to free space manager
    if (freeBlocks(entry->exts) != 0)
    {
        freePPI(&ppi);
        return -1;
    }

    // mark this directory entry unused
    entry->name[0] = '\0';
    entry->size = 0;

    // write updated parent directory back to disk
    if (writeDirectory(ppi.parentDir) != 0)
    {
        freePPI(&ppi);
        return -1;
    }

    freePPI(&ppi);
    return 0;
}