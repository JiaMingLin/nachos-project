#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#include "copyright.h"
#include "bitmap.h"
#include "synch.h"
#include "list.h"

enum EvictMethodType {

	FIFO1,
	RANDOM1,
	LRU1

};

class Lock;

class MemoryManager {

public:
	MemoryManager(int numFrames);
	MemoryManager(int numFrames,EvictMethodType replacementMethod);
	~MemoryManager();

	int AllocPage(int virtualPageNum);
	void FreePage(int physPageNum);
	bool PageIsAllocated(int physPageNum);
	void Print() { memMap->Print(); }

private:
	int numPhysPages;//now just a copy of NumPhysPages
	BitMap* memMap;
	Lock* lock;

	//used by demand paging
	//AddrSpace** spaceTable;
	int* frameTable;
	int victimPage();

	//demand paging: evicting algorithm
	EvictMethodType EvictMethod;
	List<int *> *fifoList;
};


#endif