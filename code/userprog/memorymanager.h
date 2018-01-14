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
#include "main.h"


enum ReplacementType {
	FIFO,
	RANDOM,
	LRU
};

class MemoryManager {
  public:
    MemoryManager();
    ~MemoryManager();

    void Initialize();
    void Initialize(ReplacementType method);

    int FindFrame(TranslationEntry entry);

    bool ReplaceOneWith(TranslationEntry entry);

    void FreePage(TranslationEntry entry);

    unsigned int AddrToPage(int addr);
    unsigned int Offset(int addr);

    unsigned int PageToAddr(int num);
    unsigned int PageToAddr(int num, int offset);

  private:
  	ReplacementType evictMethod;

  	// (frame, sector) pair, len = 32
  	unsigned int frameTable[NumPhysPages];

  	// (sector, -1/physFrame ) 
  	unsigned int bsTable[NumSectors]; 

  	unsigned int FindVictim();

  	void LoadSectorToMain(int secNum, int physFrame);

};

#endif
