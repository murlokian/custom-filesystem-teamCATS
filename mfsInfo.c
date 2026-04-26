/**************************************************************
 * Class::  CSC-415-03 Spring 2026
 * Name:: Patricia Ann Payumo, Abha Deshpande
 * Student IDs:: 923555591, 923314454
 * GitHub-Name::  milo-pa
 * Group-Name::  CATS
 * Project:: Basic File System
 *
 * File:: mfsInfo.c
 *
 * Description:: Implements mfs.h functions for getting metadata
 *  on files or directories
 *
 **************************************************************/

#include "mfs.h"
#include "fsDir.h"
#include "debug.h"

// return 1 if pathname exists and refers to a file otherwise 0
int fs_isFile(char *filename)
{
    parsePathInfo ppi;

    // locate target entry from the given pathname
    if (parsePath(filename, &ppi) != 0)
    {
        freePPI(&ppi);
        return -1;
    }

    // check result is valid
    if (ppi.parentDir == NULL || ppi.entryIndex < 0)
    {
        freePPI(&ppi);
        return -1;
    }

    int retVal = (ppi.parentDir[ppi.entryIndex].entryType == 0);

    freePPI(&ppi);

    // entryType: 0 = file, 1 = directory
    return retVal;
}

// return 1 if pathname exists and refers to a directory otherwise 0
int fs_isDir(char *pathname)
{
    parsePathInfo ppi;

    // locate target entry from given pathname
    if (parsePath(pathname, &ppi) != 0)
    {
        freePPI(&ppi);
        return -1;
    }

    if (ppi.parentDir == NULL || ppi.entryIndex < 0)
    {
        freePPI(&ppi);
        return -1;
    }

    int retVal = (ppi.parentDir[ppi.entryIndex].entryType == 1);

    freePPI(&ppi);

    return retVal;
}

// fs stat
// gets all the metadata about a file or a directory
// typically used when the command ls -l is used
// takes a path, looks for that directory entry, copies metadata into the buf struct
int fs_stat(const char *path, struct fs_stat *buf)
{
    DEBUG("Not implemented yet");

    // check if the path exists
    parsePathInfo ppi;

    int res = parsePath(path, &ppi);
    if (res == -1)
    {
        DEBUG("Parse Path failed: mfs.c");
        return -1;
    }

    // check if the directory exists
    if (ppi.entryIndex == -1)
    {
        DEBUG("Directory not found error: mfs.h");
        return -1;
    }
}