/**************************************************************
* Class::  CSC-415-03 Spring 2026
* Name::Abha Deshpande  
* Student IDs:: 923314454 
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

//1. 
int fs_mkdir(const char *pathname, mode_t mode){
    //NOTE: made considering there is a parsedPath struct in mfs.h as such:
    //char name[];
    //directory_entry *parent;
    //int index; //can change if plans change

    parsePathInfo s; //this is where parsepath returns a struct to mkdir
    int blockSize = VCBGlobal -> blockSize; //get the blockSize

    //called considering parsePath(char *pathname, parsedPath *s) as prototype
    int result = parsePath(pathname, &s);
    if(result == -1) return -1;


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
        strcpy(parent[i].name, s.lastElementName); //set the struct, then set the name
        break;
        }
        //if empty, what about the directory createDirectory created?
        if(empty == false){

        }
        
    
    //
    }
    //write updated parent back to disk
    LBAwrite(s.parentDir, s.parentDir[0].size / blockSize, 
        s.parentDir[0].exts[0].location);

    return 0; //mkdir returns 0 on success and -1 on failure
    
    


}
 