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
    lock = new Lock("AddrSpaceLock");
}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
// 	Dealloate an address space.
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{

    if (pageTable != NULL){

        for (int i=0;i<numPages;i++)
            if (pageTable[i].valid) {
                //kernel->tc->FreePage(pageTable[i].physicalPage);
            }

        delete[] pageTable;
    }
    delete exeFile;         // close file
    delete lock;
    //mm->Print();
}

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


void AddrSpace::pageFault(int vpn) {
    lock->Acquire();
    kernel->stats->numPageFaults++;
    
    if (!kernel->mm->ReplaceOneWith(pageTable[vpn])){
        DEBUG(dbgRobin, "Replace Page Failed, Abort");
        ASSERT(FALSE);
    }
    lock->Release();
}

bool
AddrSpace::Initialize(char *threadName)
{
    // open file
    exeFile = kernel->fileSystem->Open(threadName);
    unsigned int size;

    // validate file
    if (exeFile == NULL) {
        cerr << "Unable to open file " << threadName << "\n";
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
    
    // continueous segment data, no bubbles.
    char* dataBuf = new char[size];
    //bzero(&dataBuf[0], size);
    exeFile->ReadAt(&dataBuf[0], size, noffH.code.inFileAddr);

    // calculate page number
    numPages = divRoundUp(size, PageSize);
    size = numPages * PageSize;

    // create page table
    pageTable = new TranslationEntry[numPages];
    

    for(unsigned int i = 0; i < numPages; i++) {
        pageTable[i].virtualPage = i;
        pageTable[i].physicalPage = LoadToSector(i, dataBuf);
        pageTable[i].valid = true;
        pageTable[i].use = false;
        pageTable[i].dirty = false;
        pageTable[i].readOnly = false;
    }
    
    DEBUG(dbgRobin, "noffH.code.inFileAddr: " << noffH.code.inFileAddr);
    DEBUG(dbgRobin, "noffH.code.virtualAddr: " << noffH.code.virtualAddr);
    DEBUG(dbgRobin, "noffH.code.size: " << noffH.code.size);

    DEBUG(dbgRobin, "noffH.initData.inFileAddr: " << noffH.initData.inFileAddr);
    DEBUG(dbgRobin, "noffH.initData.virtualAddr: " << noffH.initData.virtualAddr);
    DEBUG(dbgRobin, "noffH.initData.size: " << noffH.initData.size);

    DEBUG(dbgRobin, "noffH.uninitData.inFileAddr: " << noffH.uninitData.inFileAddr);
    DEBUG(dbgRobin, "noffH.uninitData.virtualAddr: " << noffH.uninitData.virtualAddr);
    DEBUG(dbgRobin, "noffH.uninitData.size: " << noffH.uninitData.size );

    DEBUG(dbgRobin, "UserStackSize: " << UserStackSize);
    DEBUG(dbgRobin, "page number: " << numPages);

    //exeFile->ReadAt(&(kernel->machine->mainMemory[noffH.code.virtualAddr]), PageSize*5, noffH.code.inFileAddr);

    //LoadToMainMem();
    return TRUE;
}


unsigned int 
AddrSpace::LoadToSector(unsigned int virtPage, char *dataBuf){
    // find an unused sector
    int freeSector = kernel->backingStore->FindAndSet();
    if (freeSector == -1){
        // TODO: Raising exception;
        cout << "No free sectors." << endl;
        ASSERT(FALSE);
    }

    // read the executable file by using
        // target: an char array
        // size = PageSize
        // begin = segment.inFileAddr + virtPage*PageSize
    char data[PageSize];
    int virtAddr = kernel->mm->PageToAddr(virtPage);

    bcopy(&dataBuf[virtAddr], &data[0], PageSize);

    // Load to sector
    DEBUG(dbgRobin, "Writing page = " << virtPage << " to sector = " << freeSector);
    kernel->backingStore->WriteSector(freeSector, data);
    return (unsigned)freeSector;

}

void 
AddrSpace::LoadToMainMem(){
    // for each (virtPage, sector), loading frame to mainMemory by using sector.
        // read a data of length PageSize from sector.
        // copy the data to mainMemory, starting from mainMemory[addr = virtPage*PageSize]
    for (int i=0; i < numPages; i++){
        int virtPage = pageTable[i].virtualPage;
        int sectorNum = pageTable[i].physicalPage;

        int frameNum = kernel->mm->FindFrame(pageTable[i]);

        kernel->backingStore->ReadSector(sectorNum, &(kernel->machine->mainMemory[virtPage*PageSize]));
    }
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
