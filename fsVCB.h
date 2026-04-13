/**************************************************************
 * Class::  CSC-415-03 Fall 2026
 * Name:: Allie Young, Milo Pesce Ares
 * Student IDs:: 923129607, 921926821
 * GitHub-Name:: milo-pa
 * Group-Name:: CATS
 * Project:: Basic File System
 *
 * File:: fsVCB.h
 *
 * Description::  This is a header file for the volume control block.
 *
 **************************************************************/

#ifndef FSVCB_H
#define FSVCB_H

#include <stdint.h>
#include <stdbool.h>

// NOTE: signature is all of our initials -- AD, AY, MP, RM, TP :)
#define OUR_SIGNATURE 2329677687

#define VCB_BLOCK_LOCATION 0

// Volume Control Block struct used for managing the file system.  
// It is loaded when the file system is initialized and stores the metadata
// of the file system
typedef struct volume_control_block {

	// blockCount member is an unsigned 32-bit value which is the amount of blocks
	// in the volume
	uint32_t blockCount;

	// rootLocation is an unsigned 32-bit value which is the block location on disk
	// of the root directory
	uint32_t rootLocation;

	// freespaceLocation is an unsigned 32-bit value which is the block location on disk
	// of the first freespace extent  
	uint32_t freespaceLocation;

	// signature is an unsigned 32-bit value that stores the signature of
	// of the machine which formatted the volume
	uint32_t signature;

	// blockSize member is an unsigned 32-bit value which is the size of each block 
	// in bytes
	uint32_t blockSize;

} volume_control_block;

// Global instance of VCB so it can be accessed across many functions and files
// and because there is only one VCB per volume.
extern volume_control_block *VCBGlobal;

// Reads VCB from VCB_BLOCK_LOCATION on disk
// Returns pointer to VCB struct with allocated memory rounded up to blockSize
volume_control_block *loadVCB(uint32_t blockSize);

#endif
