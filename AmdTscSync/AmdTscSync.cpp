#include "AmdTscSync.h"

OSDefineMetaClassAndStructors(AmdTscSync, IOService)

// Stamp the TSC
extern "C" void stamp_tsc(void *tscp)
{
    wrmsr64(MSR_TSC, *(uint64_t*)tscp);
}

bool AmdTscSync::start(IOService *provider)
{
    bool result;
    
    IOLog("AmdTscSync: Starting...\n");
    
    result = super::start(provider);
    
    while (result)
    {
        myWorkLoop = getWorkLoop();
        
        // Create IOKit timer
        myTimer = IOTimerEventSource::timerEventSource(this, OSMemberFunctionCast(IOTimerEventSource::Action, this, &AmdTscSync::syncTSC));
        
        if (!myTimer)
        {
            IOLog("AmdTscSync: Failed to create timer event source\n");
            result = false;
            break;
        }
        
        if (myWorkLoop->addEventSource(myTimer) != kIOReturnSuccess)
        {
            IOLog("AmdTscSync: Failed to add timer event source to workloop\n");
            result = false;
            break;
        }
        
        IOLog("AmdTscSync: Syncing TSC every %d seconds.\n", SYNC_INTERVAL / 1000);
        myTimer->setTimeoutMS(SYNC_INTERVAL);
        
        break;
    }
    
    return result;
}

void AmdTscSync::stop(IOService *provider)
{
    IOLog("AmdTscSync: Stopping...\n");
    
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
void AmdTscSync::syncTSC()
{
    // Get the current TSC
    uint64_t tsc = rdtsc64();
    
#ifdef DEBUG
    IOLog("AmdTscSync: Current TSC is %lld. Rendezvousing...\n", tsc);
#endif
    
    // Call the kernel function that will call this "action" on all cores/processors
    mp_rendezvous_no_intrs(stamp_tsc, (void*)&tsc);
    
    // Reset the timer
    myTimer->setTimeoutMS(SYNC_INTERVAL);
}
