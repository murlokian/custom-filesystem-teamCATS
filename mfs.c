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

//1. 
int fs_mkdir(const char *pathname, mode_t mode){
    //NOTE: made considering there is a parsedPath struct in mfs.h as such:
    //char name[];
    //directory_entry *parent;
    //int index; //can change if plans change


    //call parsePath 
    //parsedPath s; //this is where parsepath returns a struct to mkdir

    //called considering parsePath(char *pathname, parsedPath *s) as prototype
    //int result = parsePath(pathname, &s);
    //if(result == -1) return -1;


    //get back name, parent and index
    //directory_entry *parent = s.parent;


    //Check if parent exists and isDir
    //if(s.parent == NULL) return -1;

    //call createDir(10, s.parent) -- this allocates disk space 
    //directory_entry *newDir = createDirectory(10, s.parent);

    //check if the directory was actually created
    //if(newDir == NULL) return -1;

    //for(int i= 0; i < s.parent[0].size / sizeof(directory_entry); i++){
    //loop through s.parent for name[0] = "\0"
        //if(s.parent[i].name[0] == '\0'){
        //put the new dir info in that slot
        //parent[i] = newDir[0]
        //strcpy(parent[i].name, s.name); //set the struct, then set the name
        //break}
    
    //
    //}
    //write updated parent back to disk
    //LBAwrite(s.parent, s.parent[0].size / 512, s.parent[0].exts[0].location);
    


}
//2.
char * fs_getcwd(char *pathname, size_t size){

}
//3.
int fs_setcwd(char *pathname){

}
 