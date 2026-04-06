/**************************************************************
* Class::  CSC-415-0# Spring 2024
* Name::  Allie Young, 
* Student IDs::  923129607, 
* GitHub-Name::  milo-pa
* Group-Name::  CATS
* Project:: Basic File System
*
* File:: mfs.h
*
* Description:: 
*	This is the file system interface.
*	This is the interface needed by the driver to interact with
*	your filesystem.
*
**************************************************************/

#ifndef _MFS_H
#define _MFS_H
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "b_io.h"
#include "fsFreeSpace.h"

#include <dirent.h>
#define FT_REGFILE	DT_REG
#define FT_DIRECTORY DT_DIR
#define FT_LINK	DT_LNK

// MAX_ENTRY_NAME_SIZE shall be defined as 255 in order to provide a reasonable
// file name length that is similar to other major operating systems but fills in 
// all padding
#define MAX_ENTRY_NAME_SIZE 255
#define NUM_DIR_EXTENTS       5

#ifndef uint64_t
typedef u_int64_t uint64_t;
#endif
#ifndef uint32_t
typedef u_int32_t uint32_t;
#endif

typedef struct directory_entry {

    // size member is an unsigned 64-bit value which is the size of the entry
    // measured in bytes
    uint64_t size;

    // creationDate, lastEditDate, and lastAccessDate are all time_t (8 byte)
    // that represents the number of seconds since some common epoch (tbd)
    time_t creationDate;
    time_t lastEditDate;
    time_t lastAccessDate;

    // exts is an array of extents for indexing each entry's data in memory  
    extent exts[NUM_DIR_EXTENTS];

    // entryType member is a boolean that represents whether an entry is
    // a directory or a file. 
    // 0 for file, 1 for directory
    bool entryType;

    // name member is a char array that contains the name of the entry
    // Names are unique within the scope of a directory
    // Max size of name is defined by a macro
    // if value is "\0" then the entry is unused
    char name[MAX_ENTRY_NAME_SIZE];

} directory_entry;

// This structure is returned by fs_readdir to provide the caller with information
// about each file as it iterates through a directory
struct fs_diriteminfo
	{
    unsigned short d_reclen;    /* length of this record */
    unsigned char fileType;    
    char d_name[256]; 			/* filename max filename is 255 characters */
	};

// This is a private structure used only by fs_opendir, fs_readdir, and fs_closedir
// Think of this like a file descriptor but for a directory - one can only read
// from a directory.  This structure helps you (the file system) keep track of
// which directory entry you are currently processing so that everytime the caller
// calls the function readdir, you give the next entry in the directory
typedef struct
	{
	/*****TO DO:  Fill in this structure with what your open/read directory needs  *****/
	unsigned short  d_reclen;		/* length of this record */
	unsigned short	dirEntryPosition;	/* which directory entry position, like file pos */
	//DE *	directory;			/* Pointer to the loaded directory you want to iterate */
	struct fs_diriteminfo * di;		/* Pointer to the structure you return from read */
	} fdDir;

// Key directory functions
int fs_mkdir(const char *pathname, mode_t mode);
int fs_rmdir(const char *pathname);

// Directory iteration functions
fdDir * fs_opendir(const char *pathname);
struct fs_diriteminfo *fs_readdir(fdDir *dirp);
int fs_closedir(fdDir *dirp);

// Misc directory functions
char * fs_getcwd(char *pathname, size_t size);
int fs_setcwd(char *pathname);   //linux chdir
int fs_isFile(char * filename);	//return 1 if file, 0 otherwise
int fs_isDir(char * pathname);		//return 1 if directory, 0 otherwise
int fs_delete(char* filename);	//removes a file


// This is the strucutre that is filled in from a call to fs_stat
struct fs_stat
	{
	off_t     st_size;    		/* total size, in bytes */
	blksize_t st_blksize; 		/* blocksize for file system I/O */
	blkcnt_t  st_blocks;  		/* number of 512B blocks allocated */
	time_t    st_accesstime;   	/* time of last access */
	time_t    st_modtime;   	/* time of last modification */
	time_t    st_createtime;   	/* time of last status change */
	
	/* add additional attributes here for your file system */
	};

int fs_stat(const char *path, struct fs_stat *buf);

#endif

