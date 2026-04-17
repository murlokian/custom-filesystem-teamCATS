/**************************************************************
* Class::  CSC-415-03 Spring 2026
* Name:: Abha Deshpande, Milo Pesce Ares
* Student IDs:: 923314454, 921926821
* GitHub-Name::  milo-pa
* Group-Name::  CATS
* Project:: Basic File System
*
* File:: mfs.c
*
* Description:: 
*	This is the file system interface.
*	This is the interface needed by the driver to interact with
*	your filesystem.
*
**************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "mfs.h"
#include "fsDir.h"
#include "fsVCB.h"
#include "fsDirSearch.h"
#include "fsFreeSpace.h"
#include "debug.h"

//globals for getcwd() and setcwd()
char path[MAX_PATH_SIZE];
directory_entry *curDir;

int fs_mkdir(const char *pathname, mode_t mode){
	//NOTE: made considering there is a parsedPath struct in mfs.h as such:
	//char name[];
	//directory_entry *parent;
	//int index; //can change if plans change

	parsePathInfo s; //this is where parsepath returns a struct to mkdir
	int blockSize = VCBGlobal -> blockSize; //get the blockSize

	//called considering parsePath(char *pathname, parsedPath *s) as prototype
	int result = parsePath(pathname, &s);
	if(result == -1){
		DEBUG("Parse Path failed: mfs.c");
		return -1;

	}

	//get back name, parent and index
	directory_entry *parent = s.parentDir;


	//Check if parent exists and isDir
	if(s.parentDir == NULL) return -1;

	//call createDir(10, s.parent) -- this allocates disk space 
	//bierman added 50 in class, why 50?
	directory_entry *newDir = createDirectory(10, s.parentDir);

	//check if the directory was actually created
	if(newDir == NULL) return -1;

	//flag to check if any empty slots were found 
	bool empty = false;

	for(int i= 0; i < s.parentDir[0].size / sizeof(directory_entry); i++){
	//loop through s.parent for name[0] = "\0"
		if(s.parentDir[i].name[0] == '\0'){
		//put the new dir info in that slot
		empty = true;
		parent[i] = newDir[0];
        //set the struct and then set the name
		strcpy(parent[i].name, s.lastElementName); 
		break;
		}
		//if empty, what about the directory createDirectory created?
		if(empty == false){
            free(newDir);
            newDir = NULL;
            return -1;

		}
		
	
	//
	}
	//write updated parent back to disk
	LBAwrite(s.parentDir, s.parentDir[0].size / blockSize, 
		s.parentDir[0].exts[0].location);
    
    free(newDir);
	newDir = NULL;
	return 0;
}

fdDir *fs_opendir(const char *pathname) {

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

struct fs_diriteminfo *fs_readdir(fdDir *dirp) {
	// Iterate to next entry if not at first / '.' directory  
	if(dirp->di != NULL) 
	{
		dirp->dirEntryPosition++;
	} 
	// Case at '.'  
	else 
	{
		// Allocate memory for struct to be returned to caller  
		struct fs_diriteminfo *di = malloc(sizeof(struct fs_diriteminfo));
		if(di == NULL) {
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

int fs_closedir(fdDir *dirp) {
	// Null check  
	if(dirp->di == NULL || dirp == NULL) {
		return -1;
	}

	// Free used resources  
	free(dirp->di);
	dirp->di = NULL;
	free(dirp);
	dirp = NULL;

	return 0;
}


char * fs_getcwd(char *pathname, size_t size){
    if(path[0] == '\0'){
        DEBUG("fs_getcwd() failed, initialize path first");
        return NULL;
    }
    strncpy(pathname, path, size);
    return pathname;
}

int fs_setcwd(char *pathname){
	parsePathInfo s;
    if(fs_isDir(pathname) == 0){
        DEBUG("fs_setcwd() failed, Directory does not exist error: mfs.c");
        return -1;    
    }
	//I'm copying the pathname into the path buffer provided by the parameter, 
	//and then using the defined size to only copy a certain number
    strncpy(path, pathname, MAX_PATH_SIZE);
	//then i'm using parse path to get the directory entry
	int res = parsePath(pathname, &s);
	//if parse path fails
	if(res == -1){
		DEBUG("Failed to set directory: mfs.c");
		return -1;
	}
	//thne load the directory 
	curDir = loadDir(&s.parentDir[s.entryIndex]);
	
    return 0;
}

// return 1 if pathname exists and refers to a file otherwise 0
 int fs_isFile(char * filename)
{
	parsePathInfo ppi;

	// locate target entry from the given pathname
	if (parsePath(filename, &ppi) != 0)
	{
		return 0;
	}

	// check result is valid
	if (ppi.parentDir == NULL || ppi.entryIndex < 0)
	{
		return 0;
	}

	// entryType: 0 = file, 1 = directory
	return (ppi.parentDir[ppi.entryIndex].entryType == 0);
}

// return 1 if pathname exists and refers to a directory otherwise 0
int fs_isDir(char * pathname)
{
	parsePathInfo ppi;

	// locate target entry from given pathname
	if (parsePath(pathname, &ppi) != 0)
	{
		return 0;
	}

	if (ppi.parentDir == NULL || ppi.entryIndex < 0)
	{
		return 0;
	}

	return (ppi.parentDir[ppi.entryIndex].entryType == 1);
}

// remove file by freeing its blocks and clearing its directory entry
int fs_delete(char* filename)
{
	parsePathInfo ppi;

	// find file inside parent directory
	if (parsePath(filename, &ppi) != 0)
	{
		return -1;
	}

	// deny invalid parse results
	if (ppi.parentDir == NULL || ppi.entryIndex < 0)
	{
		return -1;
	}

	directory_entry *entry = &ppi.parentDir[ppi.entryIndex];

	// only removes regular files
	if (entry->entryType != 0)
	{
		return -1;
	}

	// return file's blocks to free space manager
	if (freeBlocks(entry->exts) != 0)
	{
		return -1;
	}

	// mark this directory entry unused
	entry->name[0] = '\0';
	entry->size = 0;

	// write updated parent directory back to disk
	if (writeDirectory(ppi.parentDir) != 0)
	{
		return -1;
	}

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
		return -1;
	}

	directory_entry *entry = &ppi.parentDir[ppi.entryIndex];

	// only removes directories
	if (entry->entryType != 1)
	{
		return -1;
	}

	// load directory to check if its empty
	directory_entry *dir = loadDir(entry);
	if (dir == NULL)
	{
		return -1;
	}

	int numEntries = entry->size / sizeof(directory_entry);

	// only "." and ".." 
	for (int i = 2; i < numEntries; i++)
	{
		if (dir[i].name[0] != '\0')
		{
			free(dir);
			return -1;
		}
	}

	// freeeeeeeeeeeeee directory's blocks
	if (freeBlocks(entry->exts) != 0)
	{
		free(dir);
		return -1;
	}

	// remove this entry from parent directory
	entry->name[0] = '\0';
	entry->size = 0;

	// save updated parent directory
	if (writeDirectory(ppi.parentDir) != 0)
	{
		free(dir);
		return -1;
	}

	free(dir);
	return 0;
}