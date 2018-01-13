#include "copyright.h"
#include "main.h"


MemoryManager::MemoryManager(){
	MemoryManager(RANDOM);

}

MemoryManager::MemoryManager(ReplacementType method){
	evictMethod = method;

	for (int i = 0; i < NumPhysPages; i++){
		frameTable[i] = -1;
	}	

	for (int i = 0; i < NumSectors; i++){
		sectorTable[i] = -1;
	}

	switch(evictMethod){
		case FIFO:{
			cout<< "FIFO" << endl;
			break;
		}
		case RANDOM:{
			cout<< "RANDOM" << endl;
			break;
		}
		case LRU:{
			cout<< "LRU" << endl;
			break;
		}
		default:{
			cout << "Page replacement method not defined" << endl;
			break;
		}
	}

}

MemoryManager::~MemoryManager(){

}

unsigned int
MemoryManager::PageFindAndSet(int virtAddr){
	// addr to page number
	unsigned int virtPage = AddrToPage(virtAddr);

	// assume (page, sector is ok)
	TranslationEntry *entry;
	entry = &pageTable[vpn];
	secNum = entry->physicalPage;

	// is sec in memory?
	if (sectorTable[secNum] != -1){
		//if yes, return physAddr
		return (unsigned)sectorTable[secNum];
	}

	// page faults
	RaiseException(PageFaultException, virtAddr);

	return (unsigned)sectorTable[secNum];
}

int
MemoryManager::AddrToPage(int addr){
	return (unsigned) addr / PageSize;
}

int
MemoryManager::Offset(int addr){
	return (unsigned) addr % PageSize;
}

int
MemoryManager::PageToAddr(int num, int offset){
	return num*PageSize + offset;
}

int 
MemoryManager::FindVictim(){

	int victimPhysPage = RandomNumber() % NumPhysPages;

	int outSecNum = frameTable[victimPhysPage];

	// TODO: check the frame is dirty or not.

	// the evicted sector/frame is not in mainMemory
	sectorTable[outSecNum] = -1;

	return victimPhysPage;

}