/*  Do what you want with this. 
 This work originates from the ideas of Turbo and the 
 frustrations of cosmo1t the dell owner.
 *
 */


#include <IOKit/IOService.h>
#include <IOKit/IOLib.h>
#include <i386/proc_reg.h>



//reg define
#define MSR_IA32_TSC                    0x00000010



//extern fucntion defined in mp.c from xnu
extern "C" void  mp_rendezvous_no_intrs(
										void (*action_func)(void *),
										void *arg);
extern "C" void mp_broadcast(
			 void (*action_func)(void *),
							 void *arg);

extern "C" void rtc_clock_napped(uint64_t base, uint64_t tsc_base);

/* You can get rid of this if you want, it was used for debugging purposes testing the IOCpuNumber matching**/
//IOLock * lock;
//int countCalled;

class org_voodoo_driver_VoodooTSCSync : public IOService

{
	
	OSDeclareDefaultStructors(org_voodoo_driver_VoodooTSCSync)
private:
	void doTSC(void);
public:
	

    virtual bool init(OSDictionary *dictionary = 0);
    virtual bool start(IOService *provider);
    virtual void stop(IOService *provider);
    virtual IOReturn setPowerState(unsigned long powerStateOrdinal, IOService *whatDevice);
};
