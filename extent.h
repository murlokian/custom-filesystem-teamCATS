/**************************************************************
* Class::  CSC-415-03 Fall 2026
* Name::  Allie Young
* Student IDs::  923129607
* GitHub-Name::  milo-pa
* Group-Name::  CATS
* Project:: Basic File System
*
* File:: extent.h
*
* Description::  This is a header file for the extent struct.  
*
**************************************************************/

#ifndef EXTENT_H
#define EXTENT_H

#include <stdint.h>

typedef struct extent {  // 8 bytes
	
   // location is an unsigned 32-bit value which is the location of a starting
   // block of contiguous free space
   uint32_t location;
  
   // length is an unsigned 32-bit value which contains the 
   // total amount of bytes that this extent spans
   uint32_t length;

} extent;

#endif
