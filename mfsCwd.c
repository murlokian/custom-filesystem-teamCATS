/**************************************************************
 * Class::  CSC-415-03 Spring 2026
 * Name:: Abha Deshpande, Allie Young
 * Student IDs:: 923314454, 923129607
 * GitHub-Name::  milo-pa
 * Group-Name::  CATS
 * Project:: Basic File System
 *
 * File:: mfsCwd.c
 *
 * Description:: Implements mfs.h functions for getting and 
 *  setting current working directory
 *
 **************************************************************/

#include "mfs.h"
#include "fsDir.h"
#include "debug.h"
#include "string.h"

// globals for getcwd() and setcwd()
char path[MAX_PATH_SIZE] = "/";
directory_entry *curDir;

char *fs_getcwd(char *pathname, size_t size)
{
    if(pathname == NULL || size == 0) {
        DEBUG("invalid path buffer");
        return NULL;
    }

    if (path[0] == '\0')
    {
        DEBUG("path not initialized");
        return NULL;
    }

    strncpy(pathname, path, size);
    pathname[size - 1] = '\0';

    return pathname;
}

int fs_setcwd(char *pathname)
{
    parsePathInfo s;
    if (fs_isDir(pathname) == 0)
    {
        DEBUG("fs_setcwd() failed, Directory does not exist error: mfs.c");
        return -1;
    }
    // I'm copying the pathname into the path buffer provided by the parameter,
    // and then using the defined size to only copy a certain number
    strncpy(path, pathname, MAX_PATH_SIZE);
    // then i'm using parse path to get the directory entry
    int res = parsePath(pathname, &s);
    // if parse path fails
    if (res == -1)
    {
        DEBUG("Failed to set directory: mfs.c");
        freePPI(&s);
        return -1;
    }
    // thne load the directory
    curDir = loadDir(&s.parentDir[s.entryIndex]);
    cwdGlobal = curDir;

    freePPI(&s);

    return 0;
}
