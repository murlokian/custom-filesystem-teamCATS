/**************************************************************
* Class::  CSC-415-03 Spring 2026
* Name::  Milo Pesce Ares
* Student IDs::  921926821
* GitHub-Name::  milo-pa
* Group-Name::  CATS
* Project:: Basic File System
*
* File:: fsDirSearch.c
*
* Description:: This is the source file for helper functions
* related to searching for directory entries and parsing paths.
*
**************************************************************/

#include <string.h>

#include "fsDirSearch.h"
#include "debug.h"

int parsePath(const char * path, parsePathInfo * ppi) {
    return -1;
}

dir_index findDirEntry(const directory_entry * dir, const char * name) {

    if (dir == NULL || name == NULL) {
        return -1;
    }

    if (name[0] == '\0') {
        return -1;
    }

    // Ignore the first two entries in the directory as they are reserved for "." and ".."
    dir_index index = 2;
    directory_entry currentDir = dir[index];

    int numEntries = dir[0].size / sizeof(directory_entry);

    while (strcmp(currentDir.name, "\0") != 0) {

        index++;

        if (index >= numEntries) 
        {
            break;
        }

        if (strcmp(currentDir.name, name) == 0) 
        {
            DEBUG("findDirEntry() found '%s' in '%s' at index %d", name, dir[0].name, index);
            return index;
        }

        currentDir = dir[index];
    }

    DEBUG("findDirEntry() could not find '%s' in '%s'", name, dir[0].name);
    return -1;
}

dir_index findUnusedDirEntry(const directory_entry * dir) {
    return -1;
}
