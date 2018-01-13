// addrspace.cc 
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -n -T 0 option 
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "main.h"
#include "addrspace.h"
#include "memorymanager.h"
#include "machine.h"
#include "bitmap.h"


//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the 
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void 
SwapHeader (NoffHeader *noffH)
{
    noffH->noffMagic = WordToHost(noffH->noffMagic);
    noffH->code.size = WordToHost(noffH->code.size);
    noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
    noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
    noffH->initData.size = WordToHost(noffH->initData.size);
    noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
    noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
    noffH->uninitData.size = WordToHost(noffH->uninitData.size);
    noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
    noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Set up the translation from program memory to physical 
//	memory.  For now, this is really simple (1:1), since we are
//	only uniprogramming, and we have a single unsegmented page table
//----------------------------------------------------------------------

AddrSpace::AddrSpace()
{
    pageTable = NULL;
    backingStore = NULL;
}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
// 	Dealloate an address space.
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{

    if (pageTable != NULL){

        for (int i=0;i<numPages;i++)
            if (pageTable[i].valid) 
                kernel->tc->FreePage(pageTable[i].physicalPage);

        delete[] pageTable;
    }
    delete exeFile;         // close file

    //mm->Print();
    
    if (backingStore) delete backingStore;
}


//----------------------------------------------------------------------
// AddrSpace::Load
// 	Load a user program into memory from a file.
//
//	Assumes that the page table has been initialized, and that
//	the object code file is in NOFF format.
//
//	"fileName" is the file containing the object code to load into memory
//----------------------------------------------------------------------
/*
bool 
AddrSpace::Load(char *fileName) 
{
    OpenFile *executable = kernel->fileSystem->Open(fileName);
    NoffHeader noffH;
    unsigned int size;

    if (executable == NULL) {
	cerr << "Unable to open file " << fileName << "\n";
	return FALSE;
    }
    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) && 
		(WordToHost(noffH.noffMagic) == NOFFMAGIC))
    	SwapHeader(&noffH);
    ASSERT(noffH.noffMagic == NOFFMAGIC);

// how big is address space?
    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size 
			+ UserStackSize;	// we need to increase the size
						// to leave room for the stack

    DEBUG(dbgRobin, "noffH.code.size: " << noffH.code.size);
    DEBUG(dbgRobin, "noffH.initData.size: " << noffH.initData.size);
    DEBUG(dbgRobin, "noffH.uninitData.size: " << noffH.uninitData.size );
    DEBUG(dbgRobin, "UserStackSize: " << UserStackSize);

    numPages = divRoundUp(size, PageSize);

//	cout << "number of pages of " << fileName<< " is "<<numPages<<endl;
    size = numPages * PageSize;

    //ASSERT(usedPhysPageNum + numPages <= NumPhysPages);
                        // check we're not trying
						// to run anything too big --
						// at least until we have
						// virtual memory

    DEBUG(dbgAddr, "Initializing address space: " << numPages << ", " << size);

// Jiaming Add
    // Using linear search to find an unused physical page
    // then construct a mapping between virtual page and physical page.
    pageTable = new TranslationEntry[numPages];
    for(unsigned int i = 0, j = 0; i < numPages; i++) {
        pageTable[i].virtualPage = i;
        //while(j < NumPhysPages && AddrSpace::usedPhysPage[j] == true)
        //    j++;
        //AddrSpace::usedPhysPage[j] = true;
        pageTable[i].physicalPage = j;
        pageTable[i].valid = true;
        pageTable[i].use = false;
        pageTable[i].dirty = false;
        pageTable[i].readOnly = false;
    }

// Jiaming Add End


// then, copy in the code and data segments into memory
	if (noffH.code.size > 0) {
        DEBUG(dbgAddr, "Initializing code segment.");
        DEBUG(dbgAddr, "Virtual address: " << noffH.code.virtualAddr << ", code size: " << noffH.code.size);

        // Jiaming Add
        executable->ReadAt(
            &(kernel->machine->mainMemory[CalculateMemAddr(noffH.code.virtualAddr)]), 
            noffH.code.size, noffH.code.inFileAddr);

    }
    if (noffH.initData.size > 0) {
        DEBUG(dbgAddr, "Initializing data segment.");
        DEBUG(dbgAddr, noffH.initData.virtualAddr << ", " << noffH.initData.size);

        // Jiaming Add
        executable->ReadAt(
            &(kernel->machine->mainMemory[CalculateMemAddr(noffH.initData.virtualAddr)]),
            noffH.initData.size, noffH.initData.inFileAddr);
    }

    delete executable;			// close file
    return TRUE;			// success
}
*/

//----------------------------------------------------------------------
// AddrSpace::CalculateMemAddr
//  Calculate the address in mainMemory from the given virtualAddr in noff
//
//  "virtualAddr" is the address of code segment, initial data.
//----------------------------------------------------------------------

int
AddrSpace::CalculateMemAddr(int virtualAddr) 
{

    int physicalPage = pageTable[divRoundDown(virtualAddr, PageSize)].physicalPage;
    int offset = virtualAddr % PageSize;
    return (physicalPage * PageSize) + offset;

    //return virtualAddr;
}


int AddrSpace::pageFault(int vpn) {
    kernel->stats->numPageFaults++;
    pageTable[vpn].physicalPage = kernel->tc->memAllocate(this);
    //pageTable[vpn].physicalPage = kernel->mm->AllocPage(this,vpn);
    if (pageTable[vpn].physicalPage == -1){
        printf("Error: run out of physical memory\n");
        //to do://should yield and wait for memory space and try again?
        ASSERT(FALSE);//panic at this time
    }

    if(backingStore->PageIn(&pageTable[vpn])==-1)
        LoadPage(vpn);
    
    pageTable[vpn].valid = TRUE;
    pageTable[vpn].use = FALSE;
    pageTable[vpn].dirty = FALSE;
    //pageTable[vpn].readOnly is modified in loadPage()

    DEBUG(dbgRobin, "virtual page: "<< vpn << "; physical page: " << pageTable[vpn].physicalPage);
    
    return 0;
}

bool
AddrSpace::Initialize(char *fileName, char *threadName)
{
    // open file
    exeFile = kernel->fileSystem->Open(fileName);
    unsigned int size;

    // validate file
    if (exeFile == NULL) {
    cerr << "Unable to open file " << fileName << "\n";
    return FALSE;
    }
    exeFile->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) && 
        (WordToHost(noffH.noffMagic) == NOFFMAGIC))
        SwapHeader(&noffH);
    ASSERT(noffH.noffMagic == NOFFMAGIC);

    // calculate size
    // how big is address space?
    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size 
            + UserStackSize;    // we need to increase the size
                        // to leave room for the stack

    // calculate page number
    numPages = divRoundUp(size, PageSize);
    size = numPages * PageSize;

    // create page table
    pageTable = new TranslationEntry[numPages];
    

    for(unsigned int i = 0; i < numPages; i++) {
        pageTable[i].virtualPage = i;
        pageTable[i].physicalPage = -1;
        pageTable[i].valid = true;
        pageTable[i].use = false;
        pageTable[i].dirty = false;
        pageTable[i].readOnly = false;
    }
    DEBUG(dbgRobin, "noffH.code.size: " << noffH.code.size);
    DEBUG(dbgRobin, "noffH.initData.size: " << noffH.initData.size);
    DEBUG(dbgRobin, "noffH.uninitData.size: " << noffH.uninitData.size );
    DEBUG(dbgRobin, "UserStackSize: " << UserStackSize);
    DEBUG(dbgRobin, "page number: " << numPages);

    //exeFile->ReadAt(&(kernel->machine->mainMemory[noffH.code.virtualAddr]), PageSize*5, noffH.code.inFileAddr);

    // create backingstore
    backingStore = new BackingStore(this, numPages, threadName);

    


    return TRUE;
}

//----------------------------------------------------------------------
// AddrSpace::Execute
// 	Run a user program.  Load the executable into memory, then
//	(for now) use our own thread to run it.
//
//----------------------------------------------------------------------

void 
AddrSpace::Execute() 
{
    
    this->InitRegisters();		// set the initial register values
    this->RestoreState();		// load page table register

    kernel->machine->Run();		// jump to the user progam

    ASSERTNOTREACHED();			// machine->Run never returns;
					// the address space exits
					// by doing the syscall "exit"
}


//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters()
{
    Machine *machine = kernel->machine;
    int i;

    for (i = 0; i < NumTotalRegs; i++)
	machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);	

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

   // Set the stack register to the end of the address space, where we
   // allocated the stack; but subtract off a bit, to make sure we don't
   // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
    DEBUG(dbgAddr, "Initializing stack pointer: " << numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, don't need to save anything!
//----------------------------------------------------------------------

void AddrSpace::SaveState() 
{
        pageTable=kernel->machine->pageTable;
        numPages=kernel->machine->pageTableSize;
}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState() 
{
    kernel->machine->pageTable = pageTable;
    kernel->machine->pageTableSize = numPages;
}

//determine which segment does a address located in
//return value: 
int AddrSpace::whichSeg(int virtAddr, Segment* segPtr) {
    if (noffH.code.size > 0) {
        if (( virtAddr >= noffH.code.virtualAddr ) &&
            ( virtAddr < noffH.code.virtualAddr + noffH.code.size ))
        {
            ( *segPtr ) = noffH.code;
            return 0;
        }
    }
    if (noffH.initData.size > 0) {
        if (( virtAddr >= noffH.initData.virtualAddr ) &&
            ( virtAddr < noffH.initData.virtualAddr + noffH.initData.size ))
        {
            ( *segPtr ) = noffH.initData;
            return 1;
        }
    }
    if (noffH.uninitData.size > 0) {
        if (( virtAddr >= noffH.uninitData.virtualAddr ) &&
            ( virtAddr < noffH.uninitData.virtualAddr + noffH.uninitData.size ))
        {
            ( *segPtr ) = noffH.uninitData;
            return 2;
        }
    }
    return 3;
}

/*
Read the executable file by segment and put into mainMemory

int
AddrSpace::LoadPage(int vpn){
    DEBUG(dbgRobin, "Loading page " << vpn << " from executable file.");
    int readAddr, physAddr, size, segOffs;
    int virtAddr = vpn * PageSize;
    physAddr = pageTable[vpn].physicalPage * PageSize;

    int segBegin = noffH.code.inFileAddr + physAddr * PageSize;

    //DEBUG(dbgRobin, "Virtual address: " << noffH.code.virtualAddr << ", code size: " << noffH.code.size << ", inFileAddr: " << noffH.code.inFileAddr);
    // read a size of page.
    exeFile->ReadAt(&(kernel->machine->mainMemory[noffH.code.virtualAddr]), PageSize, segBegin);

    return 0;
}
*/

//assume there's no bubble in the exe file.
//Because when the first address in the page is not in code, initData or uninitData segment, 
//the whole page will be zero-filled

int AddrSpace::LoadPage(int vpn) {
    DEBUG(dbgRobin, "Loading page " << vpn << " from executable file.");

    int readAddr, physAddr, size, segOffs;
    int virtAddr = vpn * PageSize;
    int offs = 0;
    int ps = PageSize;
    Segment seg;
    bool readFromFile=FALSE;
    
    pageTable[vpn].readOnly = FALSE;
    do {
        physAddr = pageTable[vpn].physicalPage * PageSize + offs;
        switch (whichSeg(virtAddr, &seg)) {
        case 0://code
        {
            segOffs = virtAddr - seg.virtualAddr;
            readAddr = segOffs + seg.inFileAddr;
            size = min(ps - offs, seg.size - segOffs);
            exeFile->ReadAt(&( kernel->machine->mainMemory[physAddr] ), size, readAddr);
            readFromFile=TRUE;
            if (size==PageSize){
                pageTable[vpn].readOnly = TRUE;
            }
            if (vpn==1)
                ASSERT(kernel->machine->mainMemory[physAddr]==7);
            break;
        }
        case 1://initData
        {
            segOffs = virtAddr - seg.virtualAddr;
            readAddr = segOffs + seg.inFileAddr;
            size = min(ps - offs, seg.size - segOffs);
            exeFile->ReadAt(&( kernel->machine->mainMemory[physAddr] ), size, readAddr);
            readFromFile=TRUE;
            break;
        }
        case 2://uninitData
        {
            size = min(ps - offs, seg.size + seg.virtualAddr - virtAddr);
            bzero(&( kernel->machine->mainMemory[physAddr] ), size);
            break;
        }
        case 3://stack or others
        {
            bzero(&( kernel->machine->mainMemory[physAddr] ), PageSize - offs);
            return 0;//don't use break
        }
        }
        offs += size;
        virtAddr += size;
    } while (offs < PageSize);
    if (readFromFile)
        kernel->stats->numPageIns++;
    return 0;
}

//called by memory manager
int AddrSpace::evictPage(int physNum){
    cout << "=========================="<<endl;
    int vpn;
    for (int i = 0; i < numPages; i++){
        cout << "==========================" << i<<endl;
        if (pageTable[i].physicalPage == physNum){
            vpn = i;
            break;
        }
    }
    DEBUG(dbgRobin, "Evict Page: vpn = " << vpn << ", physNum = " << physNum);
    if (pageTable[vpn].dirty){
        backingStore->PageOut(&pageTable[vpn]);
    }
    pageTable[vpn].physicalPage = -1;
    pageTable[vpn].valid = FALSE;
    pageTable[vpn].use = FALSE;
    pageTable[vpn].dirty = FALSE;

    return 0;
}