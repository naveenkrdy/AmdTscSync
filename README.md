# VoodooTSCSync
Fork of VoodooTSCSync

A kernel extension which will synchronize the TSC on any intel cpus.

The Kext matches using IOCPUNumber in the Info.plist

so if you have a c2duo, you'd set IOCPUNumber=1 if you have an i7 you'd set IOCPUNumber=7

(the # starts @ 0)

Credit: Cosmosis Jones
