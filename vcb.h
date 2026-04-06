/**************************************************************
 * Class::  CSC-415-03 Fall 2026
 * Name::  Allie Young, Milo Pesce Ares
 * Student IDs::  923129607, 921926821
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
#include <stdbool.h>

#include "fsFreeSpace.h"

#define NUM_FREESPACE_EXTS 10
// NOTE: signature is all of our initials -- AD, AY, MP, RM, TP :)
#define OUR_SIGNATURE 2329677687

// Set at initialization
extern uint32_t blockSizeGlobal;

typedef struct volume_control_block {

    // blockCount member is an unsigned 64-bit value which is the amount of blocks
    // in the volume
    uint64_t blockCount;

    // signature is an unsigned 32-bit value that stores the signature of
    // of the machine which formatted the volume
    uint32_t signature;

    // blockSize member is an unsigned 16-bit value which is the size of each block 
    // in bytes
    uint16_t blockSize;

    // rootLocation is an unsigned 8-bit value which is the block location on disk
    // of the root directory
    uint8_t rootLocation;

    // rootLength is an unsigned 8-bit value which is the length of the root directory
    // in blocks  
    uint8_t rootLength;

    // freespaceLocation is an unsigned 8-bit value which is the block location on disk
    // of the first freespace extent  
    uint8_t freespaceLocation;

    // freespaceLength is an unsigned 8-bit value which is the length of the freespace
    // extents in blocks  
    uint8_t freespaceLength;

    // NOTE: there are 6 bytes of external padding. Could be used as a bitmap
    // to indicate file system type? Unclear if we need to support multiple
    // file systems
} volume_control_block;

// VCB will be global so we can access it across many functions and files
// and because there is only one VCB per volume.
extern volume_control_block *VCBGlobal;

// Read block 0 from disk and allocate memory for vcb
volume_control_block *loadVCB();

// Formats volume control block with NUMBLOCKS blocks, signature SIG,
// block size BLOCKSIZE, ROOTLOC as block location of the root directory,
// root directory length ROOTLEN, FSLOC as block location of first freespace extent,
// FSLEN as number of blocks for freespace map
// Initializes BLOCKSIZEGLOBAL and VCB variable
int initVCB(volume_control_block *vcb, uint64_t numBlocks, uint32_t sig, uint16_t blockSize,
            uint8_t rootLoc, uint8_t rootLen, uint8_t fsLoc, uint8_t fsLen);

// Checks if VCB is formatted using its signature
bool isVCBOurs(const volume_control_block *vcb);

#endif
