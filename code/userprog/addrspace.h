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

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"
#include <string.h>
#include "noff.h"
#include "backingstore.h"

#define UserStackSize		1024 	// increase this as necessary!


class AddrSpace {
  public:
    AddrSpace();			// Create an address space.
    ~AddrSpace();			// De-allocate an address space
    void Execute();
    
    //void Execute(char *fileName);	// Run the the program// stored in the file "executable"

    void SaveState();			// Save/restore address space-specific
    void RestoreState();		// info on a context switch 

    int pageFault(int virtualPageNum);

    bool Initialize(char *fileName, char *threadName);

    int evictPage(int vpn);

  private:
    TranslationEntry *pageTable;	// Assume linear page table translation
					// for now!
    unsigned int numPages;		// Number of pages in the virtual 
					// address space

    void InitRegisters();		// Initialize user-level CPU registers,
					// before jumping to user code

    // Jiaming Add
    int CalculateMemAddr(int virtualAddr);
    // Jiaming Add End

    // Add for demand paging
    NoffHeader noffH;
    OpenFile *exeFile;
    BackingStore* backingStore;

    int LoadPage(int virtualPageNum);     // Load the program into memory
                    // return false if not found
    int whichSeg(int virtAddr, Segment* segPtr);

};

#endif // ADDRSPACE_H
