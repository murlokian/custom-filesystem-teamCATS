/**************************************************************
* Class::  CSC-415-03 Fall 2026
* Name::  Allie Young
* Student IDs::  923129607
* GitHub-Name::  milo-pa
* Group-Name::  CATS
* Project:: Basic File System
*
* File:: vcb.c
*
* Description::  This is the volume control block struct and related functions.  
*
**************************************************************/

#include "vcb.h"
#include <stddef.h> // for returning NULL

volume_control_block* loadVCB() {
    return NULL;
}

int initVCB(volume_control_block *vcb, uint32_t sig, uint64_t numBlocks, 
    uint32_t rootLoc, uint16_t blockSize) {
    return 0;
}

bool isVCBOurs(const volume_control_block *vcb) {
    return false;
}
