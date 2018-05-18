#include "VoodooTSCSyncAMD.h"

OSDefineMetaClassAndStructors(org_voodoo_driver_VoodooTSCSyncAMD, IOService)

// Define my superclass
#define super IOService

bool org_voodoo_driver_VoodooTSCSyncAMD::start(IOService *provider)
{
    bool result;
    
    IOLog("VoodooTSCSyncAMD: Starting...\n");
    
    result = super::start(provider);
    
    while (result)
    {
        myWorkLoop = getWorkLoop();
        
        // Create IOKit timer
        myTimer = IOTimerEventSource::timerEventSource(this, OSMemberFunctionCast(IOTimerEventSource::Action, this, &org_voodoo_driver_VoodooTSCSyncAMD::syncTSC));
        
        if (!myTimer)
        {
            IOLog("VoodooTSCSyncAMD: Failed to create timer event source\n");
            result = false;
            break;
        }
        
        if (myWorkLoop->addEventSource(myTimer) != kIOReturnSuccess)
        {
            IOLog("VoodooTSCSyncAMD: Failed to add timer event source to workloop\n");
            result = false;
            break;
        }
        
        IOLog("VoodooTSCSyncAMD: Syncing TSC every %d seconds.\n", SYNC_INTERVAL / 1000);
        myTimer->setTimeoutMS(SYNC_INTERVAL);
        
        break;
    }
    
    return result;
}

void org_voodoo_driver_VoodooTSCSyncAMD::stop(IOService *provider)
{
    IOLog("VoodooTSCSyncAMD: Stopping...\n");
    
    // Clean up
    if (myTimer)
    {
        myTimer->cancelTimeout();
        myWorkLoop->removeEventSource(myTimer);
        myTimer->release();
        myTimer = NULL;
    }
    
    super::stop(provider);
}

// Update MSR on all processors
void org_voodoo_driver_VoodooTSCSyncAMD::syncTSC()
{
    // Get the current TSC
    uint64_t tsc = rdtsc64();
    
#ifdef DEBUG
    IOLog("VoodooTSCSyncAMD: Current TSC is %lld. Rendezvousing...\n", tsc);
#endif
    
    // Call the kernel function that will call this "action" on all cores/processors
    mp_rendezvous_no_intrs(stamp_tsc, (void*)&tsc);
    
    // Reset the timer
    myTimer->setTimeoutMS(SYNC_INTERVAL);
}

// Stamp the TSC
extern "C" void stamp_tsc(void *tscp)
{
    wrmsr64(MSR_TSC, *(uint64_t*)tscp);
}
