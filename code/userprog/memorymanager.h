// practice.h 
//	Data structures to keep track of executing user programs 
//	(address spaces).
//
//	For now, we don't keep any information about address spaces.
//	The user level CPU state is saved and restored in the thread
//	executing the user program (see thread.h).
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

// addrspace.h 
//	Data structures to keep track of executing user programs 
//	(address spaces).
//
//	For now, we don't keep any information about address spaces.
//	The user level CPU state is saved and restored in the thread
//	executing the user program (see thread.h).
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#include "copyright.h"
#include "machine.h"


enum ReplacementType {
	FIFO,
	RANDOM,
	LRU
};

class MemoryManager {
  public:
    MemoryManager();
    MemoryManager(ReplacementType evictMethod);
    ~MemoryManager();

    unsigned int PageFindAndSet(int virtAddr);

    unsigned int AddrToPage(int addr);
    unsigned int Offset(int addr);

    unsigned int PageToAddr(int num, int offset);

  private:
  	ReplacementType evictMethod;

  	// (frame, sector) pair, len = 32
  	int* frameTable[NumPhysPages];

  	// (sector, -1/physFrame ) 
  	int* sectorTable[NumSectors]; 

  	int FindVictim();

  	void LoadSectorToMain(int secNum, int physFrame);

};

#endif
