#include "copyright.h"
#include "main.h"
#include "addrspace.h"
#include "backingstore.h"
#include <stdio.h>
 /* Store file name for an AddrSpace backing store */
BackingStore::BackingStore(AddrSpace *as, int nPages, char *threadName) {
  // set file name to threadName
  sprintf(bsFileName, "Mem_%s", threadName);

  bsFile = NULL;
  
  this->numPages = nPages;
  valid = new bool[nPages];
  for (int i=0;i<nPages;i++){
	valid[i]=FALSE;
  }
}

/* Actually create the backing store file (on first evict) */

void
BackingStore::init() {
    kernel->fileSystem->Create(bsFileName, numPages * PageSize);
	  bsFile = kernel->fileSystem->Open(bsFileName);
}

BackingStore::~BackingStore() {
    // delete file
    kernel->fileSystem->Remove(bsFileName);
    //delete file pointer
    delete bsFile;
    delete[] bsFileName;
    delete[] valid;
}
/* Write the virtual page referenced by PTE to the backing store
   Example invocation: PageOut(&machine->pagetTable[virtualPage] or
   PageOut(&space->pageTable[virtualPage])
*/
void BackingStore::PageOut(TranslationEntry *pte) {
  
  if (bsFile==NULL){
	  init();
  }

  // virtual page n in the address space will be stored at n * PageSize in the file
  int offset = pte->virtualPage * PageSize;
  int physAddr = pte->physicalPage * PageSize;


  bsFile->WriteAt(&(kernel->machine->mainMemory[physAddr]), PageSize, offset);
  
  kernel->stats->numPageOuts++;
  
  valid[pte->virtualPage]=TRUE;
}

/* Read the virtual page referenced by PTE from the backing store */

int BackingStore::PageIn(TranslationEntry *pte) {
  DEBUG(dbgRobin, "Read the virtual page " << pte->virtualPage << "from backing store");
	if (valid[pte->virtualPage]){
		int offset = pte->virtualPage * PageSize;
		int physAddr = pte->physicalPage * PageSize;
		
		bsFile->ReadAt(&(kernel->machine->mainMemory[physAddr]), PageSize, offset);
		
		kernel->stats->numPageIns++;
		return 0;
	}else{
    DEBUG(dbgRobin, "Page " << pte->virtualPage << "is not valid.");
		return -1;
	}	
}
