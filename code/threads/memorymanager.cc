#include "copyright.h"
#include "memorymanager.h"

//#include "sysdep.h"

/* Create a manager to track the allocation of numPages of physical memory.
You will create one by calling the constructor with NumPhysPages as
the parameter.  All physical pages start as free, unallocated pages. */
MemoryManager::MemoryManager(int framePages)
{
    MemoryManager(framePages, RANDOM);
}

MemoryManager::MemoryManager(int framePages,EvictMethodType replacementMethod)
{
	/*
    DEBUG(dbgRobin, "MemoryManager initializing...");
	numPhysPages = framePages;
	memMap = new BitMap(numPhysPages);
	lock = new Lock("mapLock");

	//for demand paging
	frameTable = new int[numPhysPages];
	for(int i=0;i<numPhysPages;i++){
		frameTable[i] = -1;
	}

	//for demand paging: evicting algorithm
	EvictMethod = replacementMethod;
	switch (EvictMethod) {
	case FIFO:
		fifoList = new List<int *>();
		break;
	case RANDOM:
		break;
	case LRU:
		//int counter[numPhysPages];
		//int i;
		break;
	}
	//debug 
	//printf("##NumPhysPages=%d\n",numPages);
	*/
}

MemoryManager::~MemoryManager()
{
	/*
	delete memMap;
	delete lock;

	delete[] spaceTable;
	delete[] vpnTable;

	switch (EvictMethod) {
	case FIFO:
		delete fifoList;
		break;
	case RANDOM:
		break;
	case LRU:
		break;
	}
	*/
}

/* Allocate a free page, returning its physical page number or -1
if there are no free pages available. */
int MemoryManager::AllocPage(int virtualPageNum) {

    Print();
    DEBUG(dbgRobin, "Page Allocation...");

	int physNum = memMap->FindAndSet();
    
    /*
	if (physNum ==-1){
		
		physNum = victimPage();		
		//ask victim addrspace to evict its page
		//assuming no error		
		//spaceTable[physNum]->evictPage(vpnTable[physNum]);
		
		#if 0
		printf("phys %d, (PID %d vpn %d)\t->\t(PID %d vpn %d) \n",physNum,
			   spaceTable[physNum]->debugPid,vpnTable[physNum],
			   space->debugPid, vpn);
		#endif
	}
	
	//spaceTable[physNum] = space;
	vpnTable[physNum] = virtualPageNum;

	if (EvictMethod==FIFO)
		fifoList->Append((int*)physNum);
    
    DEBUG(dbgRobin, "Allocated physical frame number: " << physNum);
    */
	return physNum;
}

/* Free the physical page and make it available for future allocation. */
void MemoryManager::FreePage(int physPageNum){
	/*
	lock->Acquire();
	memMap->Clear(physPageNum);
	spaceTable[physPageNum] = NULL;
	vpnTable[physPageNum] = -1;
	lock->Release();
	*/
}

/* True if the physical page is allocated, false otherwise. */
bool MemoryManager::PageIsAllocated(int physPageNum) {
	/*
	lock->Acquire();
	bool physAllocated = memMap->Test(physPageNum);
	lock->Release();

	return physAllocated;
	*/
	return FALSE;
}

//return the physical page number of a victim page to be evict
int MemoryManager::victimPage() {
	//assuming lock is already acquired

	int ppn;

	switch (EvictMethod) {
	case FIFO:
		//do {
			//ASSERT(!fifoList->IsEmpty());//Otherwise it will be trapped in dead loop.
			//although impossible
		ASSERT(!fifoList->IsEmpty());
		ppn = (int)fifoList->RemoveFront();
		//} while (!memMap->Test(ppn));
		
		break;
	case RANDOM:
		ppn = RandomNumber() % numPhysPages;//change the seed using -rs
		break;
	case LRU:
		ppn = 0;
		int i;
		/*
		for (i = 1; i<numPhysPages; i++) {
			if (kernel->machine->LRUcounter[i] > kernel->machine->LRUcounter[ppn])
				ppn = i;
		}
		*/
		break;
	}

	//printf("Evicting %d\n",ppn);
	return ppn;

	return FALSE;
}