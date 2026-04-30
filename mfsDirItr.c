/**************************************************************
 * Class::  CSC-415-03 Spring 2026
 * Name:: Allie Young, Milo Pesce Ares
 * Student IDs:: 923129607, 921926821
 * GitHub-Name::  milo-pa
 * Group-Name::  CATS
 * Project:: Basic File System
 *
 * File:: mfsDirItr.c
 *
 * Description:: Implements mfs.h functions for iterating
 *  thorugh directories
 *
 **************************************************************/

#include <stdlib.h>
#include <string.h>

#include "mfs.h"
#include "debug.h"
#include "fsDir.h"

fdDir *fs_opendir(const char *pathname)
{
    parsePathInfo ppi;
    directory_entry* loadedDir;

    // Find info via given path
    if(parsePath(pathname, &ppi) == -1) {
        DEBUG("parsePath failed");
        freePPI(&ppi);
        return NULL;
    }

    // If trying to open root  
    // NOTE: a copy is loaded here instead of the global so it is not freed in closedir.  
    // NOTE: we could instead use the global and add a check in closedir? less clean imo  
    if(ppi.entryIndex == -2) {
        loadedDir = loadDir(NULL);
        if(loadedDir == NULL) {
            DEBUG("loadDir failed for root");
            freePPI(&ppi);
            return NULL;
        }
    } else {
        // Invalid parse info  
        if(ppi.parentDir == NULL || ppi.entryIndex < 0) {
            DEBUG("Invalid parse result");
            freePPI(&ppi);
            return NULL;
        }

        // Ensure target is directory
        if(ppi.parentDir[ppi.entryIndex].entryType != 1) {
            DEBUG("Target entry is not a directory");
            freePPI(&ppi);
            return NULL;
        }

        loadedDir = loadDir(&ppi.parentDir[ppi.entryIndex]);
        if(loadedDir == NULL) {
            DEBUG("loadDir failed");
            freePPI(&ppi);
            return NULL;
        }

        DEBUG("Opened directory '%s' at index %d of parent", ppi.parentDir[ppi.entryIndex].name,
        ppi.entryIndex);
    }

    // Allocate memory for directory control block
    fdDir *dirp = malloc(sizeof(fdDir));
    if (dirp == NULL)
    {
        DEBUG("malloc failed");
        freePPI(&ppi);
        return NULL;
    }

    // Set fields in directory control block
    dirp->d_reclen = sizeof(struct fs_diriteminfo);
    dirp->dirEntryPosition = 0;
    dirp->directory = loadedDir;
    dirp->di = NULL;

    freePPI(&ppi);
    return dirp;
}

struct fs_diriteminfo *fs_readdir(fdDir *dirp)
{
    // Null check  
    if(dirp == NULL || dirp->directory == NULL) {
        DEBUG("Invalid fdDir");
        return NULL;
    }

    // Iterate to next entry if not at first / '.' directory
    if(dirp->di != NULL) {
        dirp->dirEntryPosition++;
    }
    // Case at '.'
    else {
        // Allocate memory for struct to be returned to caller
        dirp->di = malloc(sizeof(struct fs_diriteminfo));
        if(dirp->di == NULL) {
            DEBUG("malloc failed");
            return NULL;
        }
        dirp->di->d_reclen = sizeof(struct fs_diriteminfo);
    }

    // Ensure not reading past last entry  
    int numEntries = dirp->directory[0].size / sizeof(directory_entry);
    while(dirp->dirEntryPosition < numEntries) {
        directory_entry currDe = dirp->directory[dirp->dirEntryPosition];

        // Skip unused entries  
        if(currDe.name[0] != '\0') {
            // Set fields in struct to be returned to caller  
            dirp->di->fileType = currDe.entryType;
            strncpy(dirp->di->d_name, currDe.name, sizeof(dirp->di->d_name) - 1);
            dirp->di->d_name[sizeof(dirp->di->d_name) - 1] = '\0';
            DEBUG("Returning entry '%s' at index %d", dirp->di->d_name, dirp->dirEntryPosition);
            return dirp->di;
        }

        dirp->dirEntryPosition++;
    }

    // No more entries  
    return NULL;
}

int fs_closedir(fdDir *dirp)
{
    // Null check
    if (dirp == NULL)
    {
        return -1;
    }

    DEBUG("Closing directory '%s'", dirp->di->d_name);

    // Free used resources
    if (dirp->di != NULL)
    {
        free(dirp->di);
        dirp->di = NULL;
    }
    if(dirp->directory != NULL) {
        free(dirp->directory);
        dirp->directory = NULL;
    } 
    free(dirp);
    dirp = NULL;

    return 0;
}
