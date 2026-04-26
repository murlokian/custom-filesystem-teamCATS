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

    // Find info via given path
    if (parsePath(pathname, &ppi))
    {
        DEBUG("parsePath failed");
        return NULL;
    }

    // Ensure target is directory
    if (ppi.parentDir[ppi.entryIndex].entryType != 1)
    {
        DEBUG("Target entry is not a directory");
        return NULL;
    }

    // Allocate memory for directory control block
    fdDir *dirp = malloc(sizeof(fdDir));
    if (dirp == NULL)
    {
        DEBUG("malloc failed in fs_opendir()");
        return NULL;
    }

    // Set fields in directory control block
    dirp->d_reclen = sizeof(struct fs_diriteminfo);
    dirp->dirEntryPosition = 0;
    dirp->directory = loadDir(&ppi.parentDir[ppi.entryIndex]);
    dirp->di = NULL;

    DEBUG("opened directory: %s", dirp->di->d_name);
    return dirp;
}

struct fs_diriteminfo *fs_readdir(fdDir *dirp)
{
    // Iterate to next entry if not at first / '.' directory
    if (dirp->di != NULL)
    {
        dirp->dirEntryPosition++;
    }
    // Case at '.'
    else
    {
        // Allocate memory for struct to be returned to caller
        struct fs_diriteminfo *di = malloc(sizeof(struct fs_diriteminfo));
        if (di == NULL)
        {
            DEBUG("malloc failed in fs_readdir()");
            free(dirp);
            dirp = NULL;
            return NULL;
        }
        dirp->di->d_reclen = sizeof(struct fs_diriteminfo);
    }

    // Set fields in struct to be returned to caller
    directory_entry currDe = dirp->directory[dirp->dirEntryPosition];
    dirp->di->fileType = currDe.entryType;
    strncpy(dirp->di->d_name, currDe.name, MAX_ENTRY_NAME_SIZE);

    return dirp->di;
}

int fs_closedir(fdDir *dirp)
{
    // Null check
    if (dirp == NULL)
    {
        return -1;
    }

    // Free used resources
    if (dirp->di != NULL)
    {
        free(dirp->di);
        dirp->di = NULL;
    }
    free(dirp);
    dirp = NULL;

    return 0;
}
