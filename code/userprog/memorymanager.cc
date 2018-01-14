#include "copyright.h"
#include "main.h"

MemoryManager::MemoryManager(){
	for (int i = 0; i < NumPhysPages; i++){
		frameTable[i] = -1;
	}	

	for (int i = 0; i < NumSectors; i++){
		bsTable[i] = -1;
	}

}

void
MemoryManager::Initialize(){
	Initialize(RANDOM);
}

void
MemoryManager::Initialize(ReplacementType method){
	evictMethod = method;

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


// Return free frame number or page fault
int
MemoryManager::FindFrame(TranslationEntry entry){
	unsigned int virtPage = entry.virtualPage;

	// assume (page, sector is ok)
	unsigned int secNum = entry.physicalPage;

	// is sec in memory?
	if (bsTable[secNum] != -1){
		//if yes, return physAddr
		DEBUG(dbgRobinDisk, "virtPage = " << virtPage << ", secNum = " << secNum << ", frame = " << bsTable[secNum]);
		return bsTable[secNum];
	}

	// is there a free frame?
	for (int i=0; i < NumPhysPages; i++){
		if (frameTable[i] == -1){
			DEBUG(dbgRobin, "Free frame = " << i << " for sector = " << secNum);
			frameTable[i] = secNum;
			bsTable[secNum] = i;

			// Load data to mainMemory
			kernel->backingStore->ReadSector(secNum, 
				&(kernel->machine->mainMemory[PageToAddr(i)]));
			return i;
		}
	}

	// page fault
	return -1;
}

bool
MemoryManager::ReplaceOneWith(TranslationEntry entry){
	// find victim
	int victimPhysPage = FindVictim();

	int victimSector = frameTable[victimPhysPage];
	
	int replaceSector = entry.physicalPage;

	// TODO: check the victimSector is dirty or not.
		// if dirty, write back to disk.
	char* dataBuf = new char[PageSize];
	bcopy(&(kernel->machine->mainMemory[PageToAddr(victimPhysPage)]), &dataBuf[0], PageSize);
	kernel->backingStore->WriteSector(victimSector, dataBuf);

	// replace the content in mainMemory from sector
	kernel->backingStore->ReadSector(replaceSector, 
		&(kernel->machine->mainMemory[PageToAddr(victimPhysPage)]));

	// update the frameTable
	frameTable[victimPhysPage] = replaceSector;

	// update the bsTable
	bsTable[victimSector] = -1;
	bsTable[replaceSector] = victimPhysPage;

	return TRUE;

}


void
MemoryManager::FreePage(TranslationEntry entry){

	// find the frame by sector
	int frameNum = bsTable[entry.physicalPage];

	// clear bsTable
	bsTable[entry.physicalPage] = -1;

	// clear frameTable
	frameTable[frameNum] = -1;
}


unsigned int
MemoryManager::AddrToPage(int addr){
	return (unsigned) addr / PageSize;
}

unsigned int
MemoryManager::Offset(int addr){
	return (unsigned) addr % PageSize;
}

unsigned int 
MemoryManager::PageToAddr(int num){
	return PageToAddr(num, 0);
}

unsigned int
MemoryManager::PageToAddr(int num, int offset){
	return (unsigned)num*PageSize + offset;
}

unsigned int 
MemoryManager::FindVictim(){

	int victimPhysPage = RandomNumber() % NumPhysPages;
	return (unsigned)victimPhysPage;

}

