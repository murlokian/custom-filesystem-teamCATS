/**************************************************************
* Class::  CSC-415-03 Fall 2026
* Name::  Allie Young
* Student IDs::  923129607
* GitHub-Name::  milo-pa
* Group-Name::  CATS
* Project:: Basic File System
*
* File:: vcb.h
*
* Description::  This is a header file for the volume control block.  
*
**************************************************************/
#ifndef VCB_H
#define VCB_H

#include <stdint.h>

#include "extent.h"

#define NUM_FREESPACE_EXTS 10

typedef struct volume_control_block {

   // blockCount member is an unsigned 64-bit value which is the amount of blocks
   // in the volume
   uint64_t blockCount;

   // freespace_exts is an array of extents for keeping track of freespace.
   // needs to be allocated and initialized when formatting volume  
   extent freespace_exts[NUM_FREESPACE_EXTS];

   // signature is an unsigned 32-bit value that stores the signature of 
   // of the machine which formatted the volume
   uint32_t signature;

   // rootLocation is an unsigned 32-bit value which is the block location on
   // disk of the root directory
   uint32_t rootLocation;

   // etreeLocation is an unsigned 32-bit value which is the block location on
   // disk of the root extent table for free space in the volume
   uint32_t etreeLocation;

   // blockSize member is an unsigned 16-bit value that represents the size
   // of a volume in bytes
   uint16_t blockSize;

   // NOTE: there are 2 bytes of external padding. Could be used as a bitmap 
   // to indicate file system type? Unclear if we need to support multiple
   // file systems
} volume_control_block;

#endif
