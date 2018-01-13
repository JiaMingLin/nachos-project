// backingstore.h

#ifndef BACKINGSTORE_H
#define BACKINGSTORE_H

#include "copyright.h"
#include "filesys.h"
#include "addrspace.h"
#include "machine.h"

class AddrSpace;

class BackingStore {
 private:
  /*
  

  */
  OpenFile *bsFile;
  int debugPid;
  int numPages;
  char bsFileName[4];
  bool *valid;
  char test[3200];
 public:
  /* Store file name for an AddrSpace backing store */

  BackingStore(AddrSpace *as, int nPages, char *threadName);
  ~BackingStore();

  /* Actually create the backing store file (on first evict) */
  void init();

  /* Write the virtual page referenced by PTE to the backing store
     Example invocation: PageOut(&machine->pagetTable[virtualPage] or
     PageOut(&space->pageTable[virtualPage])
  */
  void PageOut(TranslationEntry *pte);

  /* Read the virtual page referenced by PTE from the backing store */
  int PageIn(TranslationEntry *pte);

};


#endif