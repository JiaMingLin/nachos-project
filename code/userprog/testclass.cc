// testclass.cc 
//	Routines to choose the next thread to run, and to dispatch to
//	that thread.
//
// 	These routines assume that interrupts are already disabled.
//	If interrupts are disabled, we can assume mutual exclusion
//	(since we are on a uniprocessor).
//
// 	NOTE: We can't use Locks to provide mutual exclusion here, since
// 	if we needed to wait for a lock, and the lock was busy, we would 
//	end up calling FindNextToRun(), and that would put us in an 
//	infinite loop.
//
// 	Very simple implementation -- no priorities, straight FIFO.
//	Might need to be improved in later assignments.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "testclass.h"

TestClass::TestClass(int frameNum){
	length = frameNum;
	//memMap = new BitMap(length);
    spaceTable = new AddrSpace*[length];
	for(int i=0;i<length;i++){
		spaceTable[i] = NULL;
	}

	//ReplacementMethod = replacementMethod;
    cout << "TestClass Init" << endl;
}

TestClass::TestClass(int frameNum, ReplacementType replacementMethod){
	
}

TestClass::~TestClass(){
	delete[] spaceTable;
	//delete memMap;
}


int TestClass::FindAndSet(){
	for (int i = 0; i < length; i++){
		if (spaceTable[i] == NULL){
			return i;
		}
	}
	return -1;
}

int
TestClass::memAllocate(AddrSpace* space){

	cout << "Allocating physical memory" << endl;
	int physNum = FindAndSet();
    
    if (physNum == -1){
    	cout << "======================"<< 1 << endl;
        physNum = victimPage();
		//ask victim addrspace to evict its page
		//assuming no error
		//spaceTable[physNum]->evictPage(physNum);
	}

    spaceTable[physNum] = space;
	return physNum;
}

void
TestClass::FreePage(int physPageNum){

	//memMap->Clear(physPageNum);
	spaceTable[physPageNum] = NULL;

}

//return the physical page number of a victim page to be evict
int TestClass::victimPage() {
	//assuming lock is already acquired
	int ppn;
	cout << "======================"<< 2 << endl;
	/*
	switch (ReplacementMethod) {
	case FIFO:
		//do {
			//ASSERT(!fifoList->IsEmpty());//Otherwise it will be trapped in dead loop.
			//although impossible
		ASSERT(!fifoList->IsEmpty());
		ppn = (int)fifoList->RemoveFront();
		//} while (!memMap->Test(ppn));
		
		break;
	case RANDOM:
	    cout << "======================"<< 3 << endl;
		ppn = RandomNumber() % NumPhysPages;//change the seed using -rs
		cout << "===========ppn==========="<< ppn << endl;
		break;
	case LRU:
		ppn = 0;
		/*
		int i;

		for (i = 1; i<numPhysPages; i++) {
			if (machine->LRUcounter[i]>machine->LRUcounter[ppn])
				ppn = i;
		}
		*/
	/*
		break;
	}
*/
	ppn = RandomNumber() % NumPhysPages;
	cout << "===========ppn==========="<< ppn << endl;
	//printf("Evicting %d\n",ppn);
	return ppn;
}