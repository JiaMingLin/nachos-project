// scheduler.h 
//	Data structures for the thread dispatcher and scheduler.
//	Primarily, the list of threads that are ready to run.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef TESTCLASS_H
#define TESTCLASS_H

#include "copyright.h"
#include "list.h"
#include "bitmap.h"
#include "addrspace.h"


enum ReplacementType {
	FIFO,
	RANDOM,
	LRU
};

class TestClass {
public:
	TestClass(int frameNum);
	TestClass(int frameNum, ReplacementType replacementMethod);
	~TestClass();

	int memAllocate(AddrSpace* addrSpace);

	void FreePage(int physPageNum);

private:

    int length;
    AddrSpace** spaceTable;
    BitMap* memMap;

    int victimPage();
    int FindAndSet();

    ReplacementType ReplacementMethod;
    List<int *> *fifoList;
};

#endif