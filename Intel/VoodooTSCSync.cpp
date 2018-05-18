#include "VoodooTSCSync.h"

// Define my superclass

#define super IOService

OSDefineMetaClassAndStructors(org_voodoo_driver_VoodooTSCSync, IOService)

//stamp the tsc
extern "C"  void stamp_tsc(void *tscp) {
//	IOLockLock(lock);
//	countCalled++;
//	IOLockUnlock(lock);
	wrmsr64(MSR_IA32_TSC,*(uint64_t*)tscp);
}


bool org_voodoo_driver_VoodooTSCSync::init(OSDictionary *dictionary){
	if (!super::init(dictionary)){ return false; }

//	lock = IOLockAlloc();
//	if (lock == 0){
//		IOLog("Failed to alloc lock");
//		return false;
//	}
		
	return true;
}

static IOPMPowerState powerStates[2] = {
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {1, kIOPMPowerOn, kIOPMPowerOn, kIOPMPowerOn, 0, 0, 0, 0, 0, 0, 0, 0}
};

IOReturn org_voodoo_driver_VoodooTSCSync::setPowerState(unsigned long powerStateOrdinal, IOService *whatDevice ) {
    if (powerStateOrdinal)
        this->doTSC();
    return IOPMAckImplied;
}

void org_voodoo_driver_VoodooTSCSync::stop(IOService *provider)
{
    PMstop();
    super::stop(provider);
}


bool org_voodoo_driver_VoodooTSCSync::start(IOService *provider)

{	
	if (!super::start(provider)){ return false; }
//	countCalled = 0;
    PMinit();
    registerPowerDriver(this, powerStates, 2);
    provider->joinPMtree(this);
	return true;
	
}


/*
 Update MSR on all processors.
 */
void org_voodoo_driver_VoodooTSCSync::doTSC(){
	
	
	uint64_t tsc = rdtsc64();
	IOLog("Current tsc from rdtsc64() is %lld. Rendezvouing..\n",tsc);
	
	//now call the kernel function that will call this "action" on all cores/processors
	mp_rendezvous_no_intrs(stamp_tsc, (void*)&tsc);
	
//	IOLog("VoodooTSCSyn::doTsc(): stamp_tsc was called: %d times\n",countCalled);
	//countCalled=0;
}
