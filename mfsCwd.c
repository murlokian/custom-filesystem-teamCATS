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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

    //case1 = absolute path
    //TODO: Handle the .. cleanup
    if(pathname[0] == '/'){
        strncpy(path, pathname, MAX_PATH_SIZE);

    }
    else{
        //case 2 -- relative path
        if(!(strcmp(path, "/") == 0)){
            strcat(path, "/");
        }
        strcat(path, pathname);
    }

    //Stack appoach to clean up path
   
    char* pathStack = strdup(path);
    char *stack[MAX_PATH_SIZE];
    int top = 0;
    
    char* token = strtok(pathStack, "/");
    //1. For each token, if "..", then pop, if ".", skip, otherwise push
    while(token != NULL){
        //case 1 - ".."
        if(strcmp(token, "..") == 0){
            //pop
            top--; 
            if(top < 0){
                top = 0;
            }   
        }//case 2 - "."
        else if(strcmp(token, ".") == 0){
            //skip
        }
        else{
            //case 3 - "name"
            stack[top] = token;
            top++;

        }
        token = strtok(NULL, "/");
    }
    //2. Rebuild path from stack
    strncpy(path, "/", MAX_PATH_SIZE);
    for(int i = 0; i < top; i++){
        strcat(path, stack[i]);
        if(i < top - 1){
            strcat(path, "/");
        }
    }

    //3.free stack 
    free(pathStack);

    //parse path to get the directory entry
    int res = parsePath(pathname, &s);
    // if parse path fails
    if (res == -1)
    {
        DEBUG("Failed to set directory: mfs.c");
        freePPI(&s);
        return -1;
    }

    //edge case -- entryIndex == -2 means root directory
    if(s.entryIndex == -2){
        freePPI(&s);
        cwdGlobal = rootGlobal;
        return 0;
    }
    
    // then load the directory
    curDir = loadDir(&s.parentDir[s.entryIndex]);

    //check if curDir is null before assigning
    if(curDir == NULL){
        DEBUG("Loaddir failed : mfsCwd.c");
        return -1;
    }
    
    freePPI(&s);
    cwdGlobal = curDir;

    return 0;
}
